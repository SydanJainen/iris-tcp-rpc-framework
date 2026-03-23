#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "adapters/json_serializer.h"
#include "ports/i_serializer.h"

using iris::ISerializer;
using iris::JsonSerializer;
using nlohmann::json;

class SerializerTest : public ::testing::Test {
protected:
    std::unique_ptr<ISerializer> serializer =
        std::make_unique<JsonSerializer>();
};

TEST_F(SerializerTest, RoundTrip_SimpleObject) {
    json obj = {{"cmd", "add"}, {"args", {10, 32}}};
    auto bytes = serializer->serialize(obj);
    auto result = serializer->deserialize(bytes);
    EXPECT_EQ(result, obj);
}

TEST_F(SerializerTest, RoundTrip_Integer) {
    json obj = 42;
    auto bytes = serializer->serialize(obj);
    auto result = serializer->deserialize(bytes);
    EXPECT_EQ(result, obj);
}

TEST_F(SerializerTest, RoundTrip_String) {
    json obj = "hello world";
    auto bytes = serializer->serialize(obj);
    auto result = serializer->deserialize(bytes);
    EXPECT_EQ(result, obj);
}

TEST_F(SerializerTest, RoundTrip_Array) {
    json obj = {1, "two", 3.0, true, nullptr};
    auto bytes = serializer->serialize(obj);
    auto result = serializer->deserialize(bytes);
    EXPECT_EQ(result, obj);
}

// ---------- nested objects ----------

TEST_F(SerializerTest, RoundTrip_NestedObject) {
    json obj = {
        {"id", "abc-123"},
        {"status", "ok"},
        {"result", {
            {"values", {1, 2, 3}},
            {"metadata", {{"source", "test"}}}
        }}
    };
    auto bytes = serializer->serialize(obj);
    auto result = serializer->deserialize(bytes);
    EXPECT_EQ(result, obj);
}

// ---------- unicode ----------

TEST_F(SerializerTest, RoundTrip_Unicode) {
    json obj = {
        {"greeting", u8"こんにちは"},
        {"emoji", u8"🚀🌍"},
        {"accent", u8"café résumé"}
    };
    auto bytes = serializer->serialize(obj);
    auto result = serializer->deserialize(bytes);
    EXPECT_EQ(result, obj);
}

// ---------- empty structures ----------

TEST_F(SerializerTest, RoundTrip_EmptyObject) {
    json obj = json::object();
    auto bytes = serializer->serialize(obj);
    auto result = serializer->deserialize(bytes);
    EXPECT_TRUE(result.is_object());
    EXPECT_TRUE(result.empty());
}

TEST_F(SerializerTest, RoundTrip_EmptyArray) {
    json obj = json::array();
    auto bytes = serializer->serialize(obj);
    auto result = serializer->deserialize(bytes);
    EXPECT_TRUE(result.is_array());
    EXPECT_TRUE(result.empty());
}

// ---------- error case ----------

TEST_F(SerializerTest, DeserializeInvalidJson) {
    std::vector<uint8_t> bad = {'n', 'o', 't', 'j', 's', 'o', 'n'};
    EXPECT_THROW(serializer->deserialize(bad), std::runtime_error);
}
