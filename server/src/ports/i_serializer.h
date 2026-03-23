#ifndef IRIS_I_SERIALIZER_H
#define IRIS_I_SERIALIZER_H

#include <cstdint>
#include <vector>

#include <nlohmann/json.hpp>

namespace iris {

class ISerializer {
public:
    virtual ~ISerializer() = default;

    /**
     * Serialize an object to a byte sequence.
     *
     * @param obj The object to serialize.
     * @return UTF-8 encoded byte representation.
     */
    virtual std::vector<uint8_t> serialize(const nlohmann::json& obj) = 0;

    /**
     * Deserialize a byte sequence back into an object.
     *
     * @param data UTF-8 encoded bytes to deserialize.
     * @return The deserialized object.
     */
    virtual nlohmann::json deserialize(const std::vector<uint8_t>& data) = 0;
};

}

#endif
