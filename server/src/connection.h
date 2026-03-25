#ifndef IRIS_CONNECTION_H
#define IRIS_CONNECTION_H

#include <cstdint>
#include <stdexcept>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

namespace iris {

class Connection {
public:
    explicit Connection(SOCKET fd) : fd_(fd) {}

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&& other) noexcept : fd_(other.fd_) {
        other.fd_ = INVALID_SOCKET;
    }

    Connection& operator=(Connection&& other) noexcept {
        if (this != &other) {
            close_socket();
            fd_ = other.fd_;
            other.fd_ = INVALID_SOCKET;
        }
        return *this;
    }

    ~Connection() {
        close_socket();
    }

    void send(const std::vector<uint8_t>& data) {
        size_t total_sent = 0;
        while (total_sent < data.size()) {
            auto sent = ::send(fd_,
                               reinterpret_cast<const char*>(data.data() + total_sent),
                               static_cast<int>(data.size() - total_sent),
                               0);
            if (sent <= 0) {
                throw std::runtime_error("Connection::send: failed to send data");
            }
            total_sent += static_cast<size_t>(sent);
        }
    }

    std::vector<uint8_t> recv(size_t max_bytes) {
        std::vector<uint8_t> buffer(max_bytes);
        auto received = ::recv(fd_,
                               reinterpret_cast<char*>(buffer.data()),
                               static_cast<int>(max_bytes),
                               0);
        if (received < 0) {
            throw std::runtime_error("Connection::recv: failed to receive data");
        }
        buffer.resize(static_cast<size_t>(received));
        return buffer;
    }

    bool is_valid() const { return fd_ != INVALID_SOCKET; }
    SOCKET fd() const { return fd_; }

private:
    SOCKET fd_ = INVALID_SOCKET;

    void close_socket() {
        if (fd_ != INVALID_SOCKET) {
            ::closesocket(fd_);
            fd_ = INVALID_SOCKET;
        }
    }
};

}

#endif
