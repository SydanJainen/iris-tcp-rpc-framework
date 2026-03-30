#ifndef IRIS_WIN_TCP_LISTENER_H
#define IRIS_WIN_TCP_LISTENER_H

#include <cstdint>
#include <memory>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include "ports/i_listener.h"
#include "adapters/win/winsock_init.h"

namespace iris {

class WinTcpListener : public IListener {
public:
    WinTcpListener() = default;
    ~WinTcpListener() override;

    WinTcpListener(const WinTcpListener&) = delete;
    WinTcpListener& operator=(const WinTcpListener&) = delete;
    WinTcpListener(WinTcpListener&&) = delete;
    WinTcpListener& operator=(WinTcpListener&&) = delete;

    void start(uint16_t port) override;
    std::unique_ptr<IConnection> accept() override;
    void stop() override;

private:
    std::unique_ptr<WinsockInit> wsa_init_;
    SOCKET listen_fd_ = INVALID_SOCKET;
};

}

#endif
