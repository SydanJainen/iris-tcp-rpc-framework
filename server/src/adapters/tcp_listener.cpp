#include "tcp_listener.h"

#include <stdexcept>
#include <string>

namespace iris {

TcpListener::~TcpListener() {
    stop();
}

void TcpListener::start(uint16_t port) {
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        throw std::runtime_error(
            "TcpListener::start: WSAStartup failed with error " + std::to_string(result));
    }
    wsa_initialized_ = true;

    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd_ == INVALID_SOCKET) {
        throw std::runtime_error("TcpListener::start: failed to create socket");
    }

    int opt = 1;
    ::setsockopt(listen_fd_,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 reinterpret_cast<const char*>(&opt),
                 sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(listen_fd_,
               reinterpret_cast<sockaddr*>(&addr),
               sizeof(addr)) != 0) {
        stop();
        throw std::runtime_error(
            "TcpListener::start: failed to bind to port " + std::to_string(port));
    }

    if (::listen(listen_fd_, SOMAXCONN) != 0) {
        stop();
        throw std::runtime_error("TcpListener::start: listen() failed");
    }
}

Connection TcpListener::accept() {
    sockaddr_in client_addr{};
    int addr_len = sizeof(client_addr);
    SOCKET client_fd = ::accept(
        listen_fd_,
        reinterpret_cast<sockaddr*>(&client_addr),
        &addr_len);
    if (client_fd == INVALID_SOCKET) {
        throw std::runtime_error("TcpListener::accept: accept() failed");
    }
    return Connection(client_fd);
}

void TcpListener::stop() {
    if (listen_fd_ != INVALID_SOCKET) {
        ::closesocket(listen_fd_);
        listen_fd_ = INVALID_SOCKET;
    }
    if (wsa_initialized_) {
        WSACleanup();
        wsa_initialized_ = false;
    }
}

}
