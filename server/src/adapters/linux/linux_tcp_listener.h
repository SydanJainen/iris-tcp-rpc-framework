#ifndef IRIS_LINUX_TCP_LISTENER_H
#define IRIS_LINUX_TCP_LISTENER_H

#include <cstdint>
#include <memory>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ports/i_listener.h"

namespace iris {

class LinuxTcpListener : public IListener {
public:
    LinuxTcpListener() = default;
    ~LinuxTcpListener() override;

    LinuxTcpListener(const LinuxTcpListener&) = delete;
    LinuxTcpListener& operator=(const LinuxTcpListener&) = delete;
    LinuxTcpListener(LinuxTcpListener&&) = delete;
    LinuxTcpListener& operator=(LinuxTcpListener&&) = delete;

    void start(uint16_t port) override;
    std::unique_ptr<IConnection> accept() override;
    void stop() override;

private:
    int listen_fd_ = -1;
};

}

#endif
