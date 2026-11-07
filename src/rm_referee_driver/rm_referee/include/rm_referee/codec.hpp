#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>
#include <vector>

#include "rm_referee/commands.hpp"
#include "rm_referee/frame.hpp"

namespace rm_referee::codec
{

namespace detail
{
template <typename Cmd>
struct is_variable_length : std::false_type {};

template <>
struct is_variable_length<cmd::RobotInteraction> : std::true_type {};
}  // namespace detail

/// Number of payload bytes a command occupies on the wire.
template <typename Cmd>
std::size_t encoded_size(const Cmd & value)
{
  if constexpr (std::is_same_v<Cmd, cmd::RobotInteraction>) {
    return cmd::RobotInteraction::kHeaderLen + value.user_data.size();
  } else {
    return sizeof(Cmd);
  }
}

/// Serialise a command payload (without frame header or cmd_id).
/// Returns an empty vector for an oversized robot_interaction payload.
template <typename Cmd>
std::vector<std::uint8_t> encode(const Cmd & value)
{
  if constexpr (std::is_same_v<Cmd, cmd::RobotInteraction>) {
    if (value.user_data.size() > cmd::RobotInteraction::kMaxUserData) {
      return {};
    }
    std::vector<std::uint8_t> out(cmd::RobotInteraction::kHeaderLen + value.user_data.size());
    std::memcpy(out.data(), &value.data_cmd_id, sizeof(value.data_cmd_id));
    std::memcpy(out.data() + 2, &value.sender_id, sizeof(value.sender_id));
    std::memcpy(out.data() + 4, &value.receiver_id, sizeof(value.receiver_id));
    if (!value.user_data.empty()) {
      std::memcpy(out.data() + cmd::RobotInteraction::kHeaderLen,
        value.user_data.data(), value.user_data.size());
    }
    return out;
  } else {
    static_assert(std::is_trivially_copyable_v<Cmd>,
      "referee command payloads must be trivially copyable");
    std::vector<std::uint8_t> out(sizeof(Cmd));
    std::memcpy(out.data(), &value, sizeof(Cmd));
    return out;
  }
}

/// Deserialise a payload, checking its exact length.
template <typename Cmd>
std::optional<Cmd> decode(const std::uint8_t * data, std::size_t len)
{
  if constexpr (std::is_same_v<Cmd, cmd::RobotInteraction>) {
    if (data == nullptr || len < cmd::RobotInteraction::kHeaderLen ||
      len > cmd::RobotInteraction::kHeaderLen + cmd::RobotInteraction::kMaxUserData)
    {
      return std::nullopt;
    }
    Cmd out;
    std::memcpy(&out.data_cmd_id, data, sizeof(out.data_cmd_id));
    std::memcpy(&out.sender_id, data + 2, sizeof(out.sender_id));
    std::memcpy(&out.receiver_id, data + 4, sizeof(out.receiver_id));
    out.user_data.assign(data + cmd::RobotInteraction::kHeaderLen, data + len);
    return out;
  } else {
    static_assert(std::is_trivially_copyable_v<Cmd>,
      "referee command payloads must be trivially copyable");
    if (data == nullptr || len != sizeof(Cmd)) {
      return std::nullopt;
    }
    Cmd out;
    std::memcpy(&out, data, sizeof(Cmd));
    return out;
  }
}

template <typename Cmd>
std::optional<Cmd> decode(const Frame & frame)
{
  return decode<Cmd>(frame.payload.data(), frame.payload.size());
}

}  // namespace rm_referee::codec
