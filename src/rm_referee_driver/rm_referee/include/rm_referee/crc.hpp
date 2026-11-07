#pragma once

#include <cstddef>
#include <cstdint>

namespace rm_referee
{

/// Reflected CRC-8 as specified for the referee frame header.
/// Polynomial 0x8C (reflected), initial value 0xFF, no final XOR.
inline std::uint8_t crc8(const std::uint8_t * data, std::size_t len)
{
  std::uint8_t crc = 0xFF;
  for (std::size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x01U) ? static_cast<std::uint8_t>((crc >> 1U) ^ 0x8CU)
                          : static_cast<std::uint8_t>(crc >> 1U);
    }
  }
  return crc;
}

/// Reflected CRC-16 as specified for the referee frame tail.
/// Polynomial 0x8408 (reflected), initial value 0xFFFF, no final XOR.
inline std::uint16_t crc16(const std::uint8_t * data, std::size_t len)
{
  std::uint16_t crc = 0xFFFF;
  for (std::size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x0001U) ? static_cast<std::uint16_t>((crc >> 1U) ^ 0x8408U)
                            : static_cast<std::uint16_t>(crc >> 1U);
    }
  }
  return crc;
}

}  // namespace rm_referee
