#ifndef IRIS_JSON_SERIALIZER_H
#define IRIS_JSON_SERIALIZER_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "ports/i_serializer.h"

namespace iris {


class JsonSerializer : public ISerializer {
public:

    std::vector<uint8_t> serialize(const nlohmann::json& obj) override {
        std::string json_str = obj.dump();
        return std::vector<uint8_t>(json_str.begin(), json_str.end());
    }

    nlohmann::json deserialize(const std::vector<uint8_t>& data) override {
        try {
            std::string json_str(data.begin(), data.end());
            return nlohmann::json::parse(json_str);
        } catch (const nlohmann::json::parse_error& e) {
            throw std::runtime_error(
                std::string("JsonSerializer::deserialize: invalid JSON — ") + e.what());
        }
    }
};

}

#endif