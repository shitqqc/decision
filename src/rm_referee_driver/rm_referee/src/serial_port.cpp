#include "rm_referee/serial_port.hpp"

#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstring>

namespace rm_referee
{
namespace
{

speed_t baud_constant(std::uint32_t baudrate)
{
  switch (baudrate) {
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    case 460800: return B460800;
    case 500000: return B500000;
    case 921600: return B921600;
    case 1000000: return B1000000;
    case 1500000: return B1500000;
    case 2000000: return B2000000;
    default: return B0;
  }
}

std::string errno_message(const char * what)
{
  return std::string(what) + ": " + std::strerror(errno);
}

}  // namespace

SerialPort::~SerialPort()
{
  close();
}

bool SerialPort::is_supported_baudrate(std::uint32_t baudrate)
{
  return baud_constant(baudrate) != B0;
}

bool SerialPort::open(
  const std::string & device, std::uint32_t baudrate, std::string * error)
{
  close();

  const speed_t speed = baud_constant(baudrate);
  if (speed == B0) {
    if (error != nullptr) {
      *error = "unsupported baudrate: " + std::to_string(baudrate);
    }
    return false;
  }

  const int fd = ::open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
  if (fd < 0) {
    if (error != nullptr) {
      *error = errno_message(("open " + device).c_str());
    }
    return false;
  }

  struct termios tty;
  std::memset(&tty, 0, sizeof(tty));
  if (tcgetattr(fd, &tty) != 0) {
    if (error != nullptr) {
      *error = errno_message("tcgetattr");
    }
    ::close(fd);
    return false;
  }

  cfmakeraw(&tty);
  tty.c_cflag |= (CLOCAL | CREAD);
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 0;

  if (cfsetispeed(&tty, speed) != 0 || cfsetospeed(&tty, speed) != 0) {
    if (error != nullptr) {
      *error = errno_message("cfsetispeed/cfsetospeed");
    }
    ::close(fd);
    return false;
  }

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    if (error != nullptr) {
      *error = errno_message("tcsetattr");
    }
    ::close(fd);
    return false;
  }

  tcflush(fd, TCIOFLUSH);
  fd_.store(fd);
  return true;
}

void SerialPort::close()
{
  std::lock_guard<std::mutex> lock(write_mutex_);
  const int fd = fd_.exchange(-1);
  if (fd >= 0) {
    ::close(fd);
  }
}

std::ptrdiff_t SerialPort::read(std::uint8_t * data, std::size_t len, int timeout_ms)
{
  const int fd = fd_.load();
  if (fd < 0 || data == nullptr || len == 0) {
    return 0;
  }

  struct pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;
  pfd.revents = 0;

  const int poll_ret = ::poll(&pfd, 1, timeout_ms);
  if (poll_ret == 0) {
    return 0;
  }
  if (poll_ret < 0) {
    return errno == EINTR ? 0 : -1;
  }
  if ((pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
    return -1;
  }
  if ((pfd.revents & POLLIN) == 0) {
    return 0;
  }

  const ssize_t n = ::read(fd, data, len);
  if (n < 0) {
    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;
    }
    return -1;
  }
  return n;
}

std::ptrdiff_t SerialPort::write(
  const std::uint8_t * data, std::size_t len, std::string * error)
{
  if (data == nullptr || len == 0) {
    return 0;
  }

  std::lock_guard<std::mutex> lock(write_mutex_);
  const int fd = fd_.load();
  if (fd < 0) {
    if (error != nullptr) {
      *error = "serial port is not open";
    }
    return -1;
  }

  std::size_t written = 0;
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
  while (written < len) {
    const ssize_t n = ::write(fd, data + written, len - written);
    if (n > 0) {
      written += static_cast<std::size_t>(n);
      continue;
    }
    if (n < 0 && errno == EINTR) {
      continue;
    }
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      if (std::chrono::steady_clock::now() >= deadline) {
        if (error != nullptr) {
          *error = "serial write timed out";
        }
        return -1;
      }
      struct pollfd pfd;
      pfd.fd = fd;
      pfd.events = POLLOUT;
      pfd.revents = 0;
      ::poll(&pfd, 1, 50);
      continue;
    }
    if (error != nullptr) {
      *error = errno_message("write");
    }
    return -1;
  }
  return static_cast<std::ptrdiff_t>(written);
}

void SerialPort::flush_input()
{
  const int fd = fd_.load();
  if (fd >= 0) {
    tcflush(fd, TCIFLUSH);
  }
}

}  // namespace rm_referee
