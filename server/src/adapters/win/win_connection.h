#ifndef IRIS_WIN_CONNECTION_H
#define IRIS_WIN_CONNECTION_H

#include <cstdint>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include "ports/i_connection.h"

namespace iris {

class WinConnection : public IConnection {
public:
    explicit WinConnection(SOCKET fd);
    ~WinConnection() override;

    WinConnection(const WinConnection&) = delete;
    WinConnection& operator=(const WinConnection&) = delete;

    WinConnection(WinConnection&& other) noexcept;
    WinConnection& operator=(WinConnection&& other) noexcept;

    void send(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> recv(size_t max_bytes) override;
    bool is_valid() const override;

private:
    SOCKET fd_ = INVALID_SOCKET;

    void close_socket();
};

}

#endif
