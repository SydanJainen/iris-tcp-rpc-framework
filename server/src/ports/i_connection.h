#ifndef IRIS_I_CONNECTION_H
#define IRIS_I_CONNECTION_H

#include <cstdint>
#include <vector>

namespace iris {

class IConnection {
public:
    virtual ~IConnection() = default;

    virtual void send(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> recv(size_t max_bytes) = 0;
    virtual bool is_valid() const = 0;
};

}

#endif
