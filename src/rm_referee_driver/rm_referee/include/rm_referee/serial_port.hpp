#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

namespace rm_referee
{

/// Minimal POSIX termios wrapper for the referee system TTL link (8N1, raw,
/// no flow control). Reads are non-blocking and driven by poll().
class SerialPort
{
public:
  SerialPort() = default;
  ~SerialPort();

  SerialPort(const SerialPort &) = delete;
  SerialPort & operator=(const SerialPort &) = delete;

  /// Opens `device` at `baudrate`. On failure returns false and fills `error`.
  bool open(const std::string & device, std::uint32_t baudrate, std::string * error = nullptr);
  void close();
  bool is_open() const { return fd_.load() >= 0; }
  int fd() const { return fd_.load(); }

  /// Waits up to `timeout_ms` for data. Returns bytes read, 0 on timeout,
  /// -1 on a fatal error (the caller should close and reopen).
  std::ptrdiff_t read(std::uint8_t * data, std::size_t len, int timeout_ms);

  /// Writes the whole buffer. Returns bytes written or -1 on error.
  std::ptrdiff_t write(const std::uint8_t * data, std::size_t len, std::string * error = nullptr);

  /// Discards bytes already queued by the driver.
  void flush_input();

  static bool is_supported_baudrate(std::uint32_t baudrate);

private:
  std::atomic<int> fd_{-1};
  std::mutex write_mutex_;
};

}  // namespace rm_referee
