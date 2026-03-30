#include "adapters/win/win_connection.h"

#include <stdexcept>

namespace iris {

WinConnection::WinConnection(SOCKET fd) : fd_(fd) {}

WinConnection::~WinConnection() {
    close_socket();
}

WinConnection::WinConnection(WinConnection&& other) noexcept : fd_(other.fd_) {
    other.fd_ = INVALID_SOCKET;
}

WinConnection& WinConnection::operator=(WinConnection&& other) noexcept {
    if (this != &other) {
        close_socket();
        fd_ = other.fd_;
        other.fd_ = INVALID_SOCKET;
    }
    return *this;
}

void WinConnection::send(const std::vector<uint8_t>& data) {
    size_t total_sent = 0;
    while (total_sent < data.size()) {
        auto sent = ::send(fd_,
                           reinterpret_cast<const char*>(data.data() + total_sent),
                           static_cast<int>(data.size() - total_sent),
                           0);
        if (sent <= 0) {
            throw std::runtime_error("WinConnection::send: failed to send data");
        }
        total_sent += static_cast<size_t>(sent);
    }
}

std::vector<uint8_t> WinConnection::recv(size_t max_bytes) {
    std::vector<uint8_t> buffer(max_bytes);
    auto received = ::recv(fd_,
                           reinterpret_cast<char*>(buffer.data()),
                           static_cast<int>(max_bytes),
                           0);
    if (received < 0) {
        throw std::runtime_error("WinConnection::recv: failed to receive data");
    }
    buffer.resize(static_cast<size_t>(received));
    return buffer;
}

bool WinConnection::is_valid() const {
    return fd_ != INVALID_SOCKET;
}

void WinConnection::close_socket() {
    if (fd_ != INVALID_SOCKET) {
        ::closesocket(fd_);
        fd_ = INVALID_SOCKET;
    }
}

}
