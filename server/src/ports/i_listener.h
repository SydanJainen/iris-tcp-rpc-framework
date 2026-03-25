#ifndef IRIS_I_LISTENER_H
#define IRIS_I_LISTENER_H

#include <cstdint>
#include <memory>

namespace iris {

class Connection;

class IListener {
public:
    virtual ~IListener() = default;

    virtual void start(uint16_t port) = 0;
    virtual Connection accept() = 0;
    virtual void stop() = 0;
};

}

#endif
