#include "adapters/linux/linux_connection.h"

#include <stdexcept>

namespace iris {

LinuxConnection::LinuxConnection(int fd) : fd_(fd) {}

LinuxConnection::~LinuxConnection() {
    close_socket();
}

LinuxConnection::LinuxConnection(LinuxConnection&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

LinuxConnection& LinuxConnection::operator=(LinuxConnection&& other) noexcept {
    if (this != &other) {
        close_socket();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

void LinuxConnection::send(const std::vector<uint8_t>& data) {
    size_t total_sent = 0;
    while (total_sent < data.size()) {
        auto sent = ::send(fd_,
                           reinterpret_cast<const char*>(data.data() + total_sent),
                           data.size() - total_sent,
                           0);
        if (sent <= 0) {
            throw std::runtime_error("LinuxConnection::send: failed to send data");
        }
        total_sent += static_cast<size_t>(sent);
    }
}

std::vector<uint8_t> LinuxConnection::recv(size_t max_bytes) {
    std::vector<uint8_t> buffer(max_bytes);
    auto received = ::recv(fd_,
                           reinterpret_cast<char*>(buffer.data()),
                           max_bytes,
                           0);
    if (received < 0) {
        throw std::runtime_error("LinuxConnection::recv: failed to receive data");
    }
    buffer.resize(static_cast<size_t>(received));
    return buffer;
}

bool LinuxConnection::is_valid() const {
    return fd_ >= 0;
}

void LinuxConnection::close_socket() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

}
