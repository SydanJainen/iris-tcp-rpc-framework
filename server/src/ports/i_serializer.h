#ifndef IRIS_I_SERIALIZER_H
#define IRIS_I_SERIALIZER_H

#include <cstdint>
#include <vector>

#include <nlohmann/json.hpp>

namespace iris {

class ISerializer {
public:
    virtual ~ISerializer() = default;

    virtual std::vector<uint8_t> serialize(const nlohmann::json& obj) = 0;
    virtual nlohmann::json deserialize(const std::vector<uint8_t>& data) = 0;
};

}

#endif
