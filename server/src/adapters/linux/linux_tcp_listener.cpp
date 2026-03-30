#include "adapters/linux/linux_tcp_listener.h"

#include <stdexcept>
#include <string>

#include "adapters/linux/linux_connection.h"

namespace iris {

LinuxTcpListener::~LinuxTcpListener() {
    stop();
}

void LinuxTcpListener::start(uint16_t port) {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        throw std::runtime_error("LinuxTcpListener::start: failed to create socket");
    }

    int opt = 1;
    ::setsockopt(listen_fd_,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 &opt,
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
            "LinuxTcpListener::start: failed to bind to port " + std::to_string(port));
    }

    if (::listen(listen_fd_, SOMAXCONN) != 0) {
        stop();
        throw std::runtime_error("LinuxTcpListener::start: listen() failed");
    }
}

std::unique_ptr<IConnection> LinuxTcpListener::accept() {
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(
        listen_fd_,
        reinterpret_cast<sockaddr*>(&client_addr),
        &addr_len);
    if (client_fd < 0) {
        throw std::runtime_error("LinuxTcpListener::accept: accept() failed");
    }
    return std::make_unique<LinuxConnection>(client_fd);
}

void LinuxTcpListener::stop() {
    if (listen_fd_ >= 0) {
        ::close(listen_fd_);
        listen_fd_ = -1;
    }
}

}
