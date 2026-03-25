#ifndef IRIS_TCP_LISTENER_H
#define IRIS_TCP_LISTENER_H

#include <cstdint>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "ports/i_listener.h"
#include "connection.h"

namespace iris {

class TcpListener : public IListener {
public:
    TcpListener() = default;
    ~TcpListener() override;

    TcpListener(const TcpListener&) = delete;
    TcpListener& operator=(const TcpListener&) = delete;

    void start(uint16_t port) override;
    Connection accept() override;
    void stop() override;

private:
    SOCKET listen_fd_ = INVALID_SOCKET;
    bool wsa_initialized_ = false;
};

}

#endif
