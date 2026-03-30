#ifndef IRIS_WINSOCK_INIT_H
#define IRIS_WINSOCK_INIT_H

#include <stdexcept>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

namespace iris {

class WinsockInit {
public:
    WinsockInit() {
        WSADATA wsa_data;
        int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (result != 0) {
            throw std::runtime_error(
                "WinsockInit: WSAStartup failed with error " + std::to_string(result));
        }
    }

    ~WinsockInit() {
        WSACleanup();
    }

    WinsockInit(const WinsockInit&) = delete;
    WinsockInit& operator=(const WinsockInit&) = delete;
    WinsockInit(WinsockInit&&) = delete;
    WinsockInit& operator=(WinsockInit&&) = delete;
};

}

#endif
