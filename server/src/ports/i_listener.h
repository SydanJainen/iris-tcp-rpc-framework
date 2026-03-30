#ifndef IRIS_I_LISTENER_H
#define IRIS_I_LISTENER_H

#include <cstdint>
#include <memory>

#include "ports/i_connection.h"

namespace iris {

class IListener {
public:
    virtual ~IListener() = default;

    virtual void start(uint16_t port) = 0;
    virtual std::unique_ptr<IConnection> accept() = 0;
    virtual void stop() = 0;
};

}

#endif
