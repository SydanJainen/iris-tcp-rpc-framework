#ifndef IRIS_LENGTH_PREFIXED_FRAMER_H
#define IRIS_LENGTH_PREFIXED_FRAMER_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

#include "ports/i_framer.h"

namespace iris {

class LengthPrefixedFramer : public IFramer {
public:

    std::vector<uint8_t> pack(const std::vector<uint8_t>& payload) override {
        uint32_t length = static_cast<uint32_t>(payload.size());

        std::vector<uint8_t> frame;
        frame.reserve(4 + payload.size());

        frame.push_back(static_cast<uint8_t>((length >> 24) & 0xFF));
        frame.push_back(static_cast<uint8_t>((length >> 16) & 0xFF));
        frame.push_back(static_cast<uint8_t>((length >> 8) & 0xFF));
        frame.push_back(static_cast<uint8_t>(length & 0xFF));

        frame.insert(frame.end(), payload.begin(), payload.end());
        return frame;
    }

    std::vector<uint8_t> unpack(const std::vector<uint8_t>& stream) override {
        if (stream.size() < 4) {
            throw std::runtime_error(
                "LengthPrefixedFramer::unpack: stream too short for header "
                "(need 4 bytes, got " + std::to_string(stream.size()) + ")");
        }

        uint32_t length = (static_cast<uint32_t>(stream[0]) << 24)
                        | (static_cast<uint32_t>(stream[1]) << 16)
                        | (static_cast<uint32_t>(stream[2]) << 8)
                        | (static_cast<uint32_t>(stream[3]));

        if (stream.size() < 4 + length) {
            throw std::runtime_error(
                "LengthPrefixedFramer::unpack: stream too short for payload "
                "(header has " + std::to_string(length) + " bytes, but only "
                + std::to_string(stream.size() - 4) + " available)");
        }

        return std::vector<uint8_t>(stream.begin() + 4,
                                     stream.begin() + 4 + length);
    }
};

}

#endif
