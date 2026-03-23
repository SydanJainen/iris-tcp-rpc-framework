#ifndef IRIS_I_FRAMER_H
#define IRIS_I_FRAMER_H

#include <cstdint>
#include <vector>

namespace iris {

    class IFramer {
public:
    virtual ~IFramer() = default;

    /**
     * Pack a payload by prepending framing metadata.
     *
     * @param payload Raw payload bytes to frame.
     * @return Framed bytes (header + payload), ready for transmission.
     */
    virtual std::vector<uint8_t> pack(const std::vector<uint8_t>& payload) = 0;

    /**
     * Unpack a framed message, extracting the payload.
     *
     * @param stream Raw bytes received from the wire, including the
     *               framing header and the payload.
     * @return The payload bytes without framing metadata.
     */
    virtual std::vector<uint8_t> unpack(const std::vector<uint8_t>& stream) = 0;
};

}

#endif
