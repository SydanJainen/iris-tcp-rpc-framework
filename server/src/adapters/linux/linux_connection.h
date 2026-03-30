#ifndef IRIS_LINUX_CONNECTION_H
#define IRIS_LINUX_CONNECTION_H

#include <cstdint>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>

#include "ports/i_connection.h"

namespace iris {

class LinuxConnection : public IConnection {
public:
    explicit LinuxConnection(int fd);
    ~LinuxConnection() override;

    LinuxConnection(const LinuxConnection&) = delete;
    LinuxConnection& operator=(const LinuxConnection&) = delete;

    LinuxConnection(LinuxConnection&& other) noexcept;
    LinuxConnection& operator=(LinuxConnection&& other) noexcept;

    void send(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> recv(size_t max_bytes) override;
    bool is_valid() const override;

private:
    int fd_ = -1;

    void close_socket();
};

}

#endif
