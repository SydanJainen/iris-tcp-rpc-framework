#ifndef IRIS_I_FRAMER_H
#define IRIS_I_FRAMER_H

#include <cstdint>
#include <vector>

namespace iris {

    class IFramer {
public:
    virtual ~IFramer() = default;

    virtual std::vector<uint8_t> pack(const std::vector<uint8_t>& payload) = 0;
    virtual std::vector<uint8_t> unpack(const std::vector<uint8_t>& stream) = 0;
};

}

#endif
