#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "rm_referee/crc.hpp"

namespace rm_referee
{

inline constexpr std::uint8_t kSof = 0xA5;
inline constexpr std::size_t kHeaderSize = 5;   // SOF + length + seq + CRC8
inline constexpr std::size_t kCmdIdSize = 2;
inline constexpr std::size_t kTailSize = 2;     // CRC16
inline constexpr std::size_t kFrameOverhead = kHeaderSize + kCmdIdSize + kTailSize;  // 9
inline constexpr std::size_t kMaxPayloadSize = 118;  // 0x0301 with 112-byte user data

/// A validated referee frame. The payload is the raw command data (without cmd_id).
struct Frame
{
  std::uint16_t cmd_id{0};
  std::uint8_t seq{0};
  std::vector<std::uint8_t> payload;
};

struct ParserStats
{
  std::uint64_t frames{0};
  std::uint64_t crc8_errors{0};
  std::uint64_t crc16_errors{0};
  std::uint64_t oversize_frames{0};
  std::uint64_t dropped_bytes{0};
};

/// Build a complete frame: header + cmd_id + payload + trailing CRC16.
/// Returns an empty vector if the payload cannot be represented.
inline std::vector<std::uint8_t> build_frame(
  std::uint16_t cmd_id, const std::uint8_t * payload, std::size_t payload_len,
  std::uint8_t seq)
{
  if (payload_len > 0xFFFFU || (payload_len > 0 && payload == nullptr)) {
    return {};
  }

  std::vector<std::uint8_t> frame;
  frame.reserve(kFrameOverhead + payload_len);
  frame.push_back(kSof);
  frame.push_back(static_cast<std::uint8_t>(payload_len & 0xFFU));
  frame.push_back(static_cast<std::uint8_t>((payload_len >> 8U) & 0xFFU));
  frame.push_back(seq);
  frame.push_back(crc8(frame.data(), frame.size()));

  frame.push_back(static_cast<std::uint8_t>(cmd_id & 0xFFU));
  frame.push_back(static_cast<std::uint8_t>((cmd_id >> 8U) & 0xFFU));
  if (payload_len > 0) {
    frame.insert(frame.end(), payload, payload + payload_len);
  }

  const std::uint16_t tail = crc16(frame.data(), frame.size());
  frame.push_back(static_cast<std::uint8_t>(tail & 0xFFU));
  frame.push_back(static_cast<std::uint8_t>((tail >> 8U) & 0xFFU));
  return frame;
}

inline std::vector<std::uint8_t> build_frame(
  std::uint16_t cmd_id, const std::vector<std::uint8_t> & payload, std::uint8_t seq)
{
  return build_frame(cmd_id, payload.data(), payload.size(), seq);
}

/// Incremental stream parser. It resynchronises on 0xA5 and validates both CRCs.
/// Not thread-safe: feed it from one thread only.
class FrameParser
{
public:
  using Callback = std::function<void(Frame &&)>;

  explicit FrameParser(std::size_t max_payload = kMaxPayloadSize)
  {
    set_max_payload(max_payload);
  }

  /// Upper bound for a single payload. Standard-link traffic never exceeds 118 B;
  /// the cap also protects against a corrupted length field.
  void set_max_payload(std::size_t max_payload)
  {
    max_payload_ = max_payload < 1 ? 1 : (max_payload > 4096 ? 4096 : max_payload);
  }

  std::size_t max_payload() const { return max_payload_; }

  void reset()
  {
    buffer_.clear();
    stats_ = ParserStats{};
  }

  const ParserStats & stats() const { return stats_; }

  void feed(const std::uint8_t * data, std::size_t len, const Callback & callback)
  {
    if (data == nullptr || len == 0) {
      return;
    }
    buffer_.insert(buffer_.end(), data, data + len);
    process(callback);
  }

  void feed(const std::vector<std::uint8_t> & data, const Callback & callback)
  {
    feed(data.data(), data.size(), callback);
  }

private:
  void process(const Callback & callback)
  {
    while (true) {
      // Drop everything before the next start-of-frame byte.
      std::size_t sof = 0;
      while (sof < buffer_.size() && buffer_[sof] != kSof) {
        ++sof;
      }
      if (sof > 0) {
        stats_.dropped_bytes += sof;
        buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<std::ptrdiff_t>(sof));
      }
      if (buffer_.size() < kHeaderSize) {
        return;
      }

      const std::size_t payload_len =
        static_cast<std::size_t>(buffer_[1]) | (static_cast<std::size_t>(buffer_[2]) << 8U);

      if (payload_len > max_payload_) {
        ++stats_.oversize_frames;
        ++stats_.dropped_bytes;
        buffer_.erase(buffer_.begin());
        continue;
      }

      const std::size_t frame_len = kFrameOverhead + payload_len;
      if (buffer_.size() < frame_len) {
        return;  // wait for more bytes
      }

      if (crc8(buffer_.data(), 4) != buffer_[4]) {
        ++stats_.crc8_errors;
        ++stats_.dropped_bytes;
        buffer_.erase(buffer_.begin());
        continue;
      }

      const std::uint16_t expected_tail =
        static_cast<std::uint16_t>(buffer_[frame_len - 2]) |
        (static_cast<std::uint16_t>(buffer_[frame_len - 1]) << 8U);
      if (crc16(buffer_.data(), frame_len - 2) != expected_tail) {
        ++stats_.crc16_errors;
        ++stats_.dropped_bytes;
        buffer_.erase(buffer_.begin());
        continue;
      }

      Frame frame;
      frame.cmd_id = static_cast<std::uint16_t>(buffer_[5]) |
        (static_cast<std::uint16_t>(buffer_[6]) << 8U);
      frame.seq = buffer_[3];
      frame.payload.assign(
        buffer_.begin() + static_cast<std::ptrdiff_t>(kHeaderSize + kCmdIdSize),
        buffer_.begin() + static_cast<std::ptrdiff_t>(frame_len - kTailSize));

      buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<std::ptrdiff_t>(frame_len));
      ++stats_.frames;
      callback(std::move(frame));
    }
  }

  std::vector<std::uint8_t> buffer_;
  std::size_t max_payload_{kMaxPayloadSize};
  ParserStats stats_{};
};

}  // namespace rm_referee
