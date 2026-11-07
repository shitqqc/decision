#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include "rm_referee/codec.hpp"
#include "rm_referee/frame.hpp"
#include "rm_referee_msgs/msg/referee_frame.hpp"
#include "rm_referee/serial_port.hpp"

namespace rm_referee
{

/// Compile-time list of commands a node wants to decode and dispatch.
template <typename... Commands>
struct CommandList
{};

namespace detail
{

template <typename Derived, typename Cmd, typename = void>
struct has_on_command : std::false_type
{};

template <typename Derived, typename Cmd>
struct has_on_command<Derived, Cmd, std::void_t<decltype(
    std::declval<Derived &>().on_command(std::declval<const Cmd &>()))>> : std::true_type
{};

template <typename Derived, typename = void>
struct has_on_frame : std::false_type
{};

template <typename Derived>
struct has_on_frame<Derived, std::void_t<decltype(
    std::declval<Derived &>().on_frame(std::declval<const Frame &>()))>> : std::true_type
{};

}  // namespace detail

/// CRTP base class for a referee-system node.
///
/// `Derived` is the concrete node type and `CommandList<...>` names exactly the
/// commands that are decoded and dispatched to it. The base owns the TTL port,
/// validates frames, publishes every selected frame on `~/raw_frame`, and calls
/// `Derived::on_command(const Cmd &)` for every selected command type that has
/// such a method. A command can be listed without a handler (raw output only).
///
/// The derived class must call `start()` (typically at the end of its
/// constructor) to launch the receive thread. Handlers run on that thread and
/// are not serialised with the ROS executor. Subscriptions live on the ROS
/// executor, so `send()` is safe to call from either.
template <typename Derived, typename List>
class RefereeNode;

template <typename Derived, typename... Cmds>
class RefereeNode<Derived, CommandList<Cmds...>> : public rclcpp::Node
{
public:
  using FrameCallback = std::function<void(const Frame &)>;

  explicit RefereeNode(
    const std::string & node_name,
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : rclcpp::Node(node_name, options)
  {
    port_ = declare_parameter<std::string>("port", "/dev/ttyUSB0");
    baudrate_ = static_cast<std::uint32_t>(
      declare_parameter<std::int64_t>("baudrate", 115200));
    read_timeout_ms_ = static_cast<int>(declare_parameter<std::int64_t>("read_timeout_ms", 50));
    reconnect_interval_ms_ =
      static_cast<int>(declare_parameter<std::int64_t>("reconnect_interval_ms", 1000));
    const auto max_payload =
      declare_parameter<std::int64_t>("max_payload_size", static_cast<std::int64_t>(kMaxPayloadSize));
    publish_raw_ = declare_parameter<bool>("publish_raw", false);
    frame_id_ = declare_parameter<std::string>("frame_id", "referee");

    if (read_timeout_ms_ < 0) {
      read_timeout_ms_ = 0;
    }
    if (reconnect_interval_ms_ < 10) {
      reconnect_interval_ms_ = 10;
    }
    max_payload_size_ =
      static_cast<std::size_t>(std::clamp<std::int64_t>(max_payload, 1, 4096));
    parser_.set_max_payload(max_payload_size_);

    raw_pub_ = create_publisher<rm_referee_msgs::msg::RefereeFrame>("~/raw_frame", rclcpp::QoS(50));
    connected_pub_ = create_publisher<std_msgs::msg::Bool>(
      "~/connected", rclcpp::QoS(1).transient_local());
    connected_pub_->publish(std_msgs::msg::Bool{});
  }

  ~RefereeNode() override
  {
    stop();
  }

  RefereeNode(const RefereeNode &) = delete;
  RefereeNode & operator=(const RefereeNode &) = delete;

  /// Starts the serial receive thread. Idempotent.
  void start()
  {
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) {
      return;
    }
    thread_ = std::thread([this]() { rx_loop(); });
  }

  /// Stops the receive thread and closes the port. Idempotent.
  void stop()
  {
    running_.store(false);
    if (thread_.joinable()) {
      if (thread_.get_id() == std::this_thread::get_id()) {
        RCLCPP_WARN(get_logger(), "stop() called from the receive thread; detaching is unsafe");
        return;
      }
      thread_.join();
    }
    serial_.close();
    set_connected(false);
  }

  bool is_running() const { return running_.load(); }

  /// True while the TTY is open. This says nothing about the referee system
  /// being powered: the link is receive-mostly, so use frame traffic as the
  /// real liveness indicator.
  bool is_connected() const { return connected_.load(); }

  const std::string & port() const { return port_; }
  std::uint32_t baudrate() const { return baudrate_; }

  /// Sends one raw command payload. Thread-safe.
  bool send_raw(std::uint16_t cmd_id, const std::uint8_t * data, std::size_t len)
  {
    if (len > max_payload_size_) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 2000,
        "referee payload of %zu bytes exceeds max_payload_size (%zu)", len, max_payload_size_);
      return false;
    }

    std::lock_guard<std::mutex> lock(tx_mutex_);
    const std::uint8_t seq = tx_seq_++;
    const std::vector<std::uint8_t> frame = build_frame(cmd_id, data, len, seq);
    if (frame.empty()) {
      RCLCPP_WARN(get_logger(), "failed to build frame for command 0x%04X", cmd_id);
      return false;
    }

    std::string error;
    const std::ptrdiff_t written = serial_.write(frame.data(), frame.size(), &error);
    if (written != static_cast<std::ptrdiff_t>(frame.size())) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 2000, "failed to send command 0x%04X: %s",
        cmd_id, error.empty() ? "short write" : error.c_str());
      return false;
    }
    return true;
  }

  /// Serialises and sends a typed command. The command type must be one of the
  /// protocol payload structs carrying a `kId` member.
  template <typename Cmd>
  bool send(const Cmd & command)
  {
    const std::vector<std::uint8_t> payload = codec::encode(command);
    if (payload.size() != codec::encoded_size(command)) {
      RCLCPP_WARN(
        get_logger(), "refusing to send command 0x%04X: invalid payload length", Cmd::kId);
      return false;
    }
    return send_raw(Cmd::kId, payload.data(), payload.size());
  }

  /// Optional hook invoked for every valid frame before typed dispatch.
  /// May be called from any thread; the callback itself runs on the receive
  /// thread (or on the caller's thread when using feed()).
  void set_frame_callback(FrameCallback callback)
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    frame_callback_ = std::move(callback);
  }

  /// Feeds bytes through the normal parser and dispatch path without touching
  /// the serial port. Intended for tests and offline replay of recorded data.
  /// Do not call while the receive thread is running.
  void feed(const std::uint8_t * data, std::size_t len)
  {
    if (running_.load()) {
      RCLCPP_WARN(get_logger(), "feed() ignored while the receive thread is running");
      return;
    }
    parser_.feed(data, len, [this](Frame && frame) { handle_frame(std::move(frame)); });
  }

  void feed(const std::vector<std::uint8_t> & data)
  {
    feed(data.data(), data.size());
  }

protected:
  /// Fills a message header with the receive timestamp and configured frame id.
  std_msgs::msg::Header make_header(const rclcpp::Time & stamp) const
  {
    std_msgs::msg::Header header;
    header.stamp = stamp;
    header.frame_id = frame_id_;
    return header;
  }

  const std::string & frame_id() const { return frame_id_; }

private:
  void rx_loop()
  {
    while (running_.load()) {
      if (!serial_.is_open()) {
        std::string error;
        if (!serial_.open(port_, baudrate_, &error)) {
          RCLCPP_ERROR_THROTTLE(
            get_logger(), *get_clock(), 5000,
            "cannot open referee port %s @ %u: %s", port_.c_str(), baudrate_, error.c_str());
          std::this_thread::sleep_for(std::chrono::milliseconds(reconnect_interval_ms_));
          continue;
        }
        parser_.reset();
        set_connected(true);
        RCLCPP_INFO(
          get_logger(), "referee link open on %s @ %u", port_.c_str(), baudrate_);
      }

      std::uint8_t buffer[512];
      const std::ptrdiff_t n = serial_.read(buffer, sizeof(buffer), read_timeout_ms_);
      if (n > 0) {
        parser_.feed(buffer, static_cast<std::size_t>(n),
          [this](Frame && frame) { handle_frame(std::move(frame)); });
      } else if (n < 0) {
        RCLCPP_WARN(
          get_logger(), "referee link lost on %s; reopening", port_.c_str());
        serial_.close();
        set_connected(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(reconnect_interval_ms_));
      }
    }
  }

  void handle_frame(Frame && frame)
  {
    const rclcpp::Time stamp = now();

    if (publish_raw_) {
      rm_referee_msgs::msg::RefereeFrame raw;
      raw.header = make_header(stamp);
      raw.cmd_id = frame.cmd_id;
      raw.seq = frame.seq;
      raw.data = frame.payload;
      raw_pub_->publish(raw);
    }

    if constexpr (detail::has_on_frame<Derived>::value) {
      static_cast<Derived *>(this)->on_frame(frame);
    }
    FrameCallback callback;
    {
      std::lock_guard<std::mutex> lock(callback_mutex_);
      callback = frame_callback_;
    }
    if (callback) {
      callback(frame);
    }

    const bool handled = (dispatch_one<Cmds>(frame) || ...);
    if (!handled) {
      RCLCPP_DEBUG(get_logger(), "unhandled referee command 0x%04X", frame.cmd_id);
    }
  }

  template <typename Cmd>
  bool dispatch_one(const Frame & frame)
  {
    if (frame.cmd_id != Cmd::kId) {
      return false;
    }
    if constexpr (detail::has_on_command<Derived, Cmd>::value) {
      auto decoded = codec::decode<Cmd>(frame);
      if (!decoded) {
        RCLCPP_WARN_THROTTLE(
          get_logger(), *get_clock(), 2000,
          "command 0x%04X has %zu payload bytes, expected a different length",
          frame.cmd_id, frame.payload.size());
        return true;
      }
      static_cast<Derived *>(this)->on_command(*decoded);
    }
    return true;
  }

  void set_connected(bool connected)
  {
    connected_.store(connected);
    std_msgs::msg::Bool msg;
    msg.data = connected;
    connected_pub_->publish(msg);
  }

  std::string port_;
  std::uint32_t baudrate_{115200};
  int read_timeout_ms_{50};
  int reconnect_interval_ms_{1000};
  std::size_t max_payload_size_{kMaxPayloadSize};
  bool publish_raw_{false};
  std::string frame_id_;

  SerialPort serial_;
  FrameParser parser_;
  std::atomic<bool> running_{false};
  std::atomic<bool> connected_{false};
  std::thread thread_;

  std::mutex tx_mutex_;
  std::uint8_t tx_seq_{0};

  std::mutex callback_mutex_;
  FrameCallback frame_callback_;
  rclcpp::Publisher<rm_referee_msgs::msg::RefereeFrame>::SharedPtr raw_pub_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr connected_pub_;
};

template <typename Derived, typename... Commands>
using RefereeNodeFor = RefereeNode<Derived, CommandList<Commands...>>;

}  // namespace rm_referee
