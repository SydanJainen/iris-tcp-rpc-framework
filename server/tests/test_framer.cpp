#include <cstdint>
#include <memory>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

#include "adapters/length_prefixed_framer.h"
#include "ports/i_framer.h"

using iris::IFramer;
using iris::LengthPrefixedFramer;

class FramerTest : public ::testing::Test {
protected:
    std::unique_ptr<IFramer> framer = std::make_unique<LengthPrefixedFramer>();
};

TEST_F(FramerTest, PackUnpackSymmetry_SmallPayload) {
    std::vector<uint8_t> payload = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  // "Hello"
    auto packed = framer->pack(payload);
    auto unpacked = framer->unpack(packed);
    EXPECT_EQ(unpacked, payload);
}

TEST_F(FramerTest, PackUnpackSymmetry_SingleByte) {
    std::vector<uint8_t> payload = {0xFF};
    auto packed = framer->pack(payload);
    auto unpacked = framer->unpack(packed);
    EXPECT_EQ(unpacked, payload);
}

TEST_F(FramerTest, PackUnpackSymmetry_MediumPayload) {
    std::vector<uint8_t> payload(256);
    std::iota(payload.begin(), payload.end(), 0);  // 0x00..0xFF
    auto packed = framer->pack(payload);
    auto unpacked = framer->unpack(packed);
    EXPECT_EQ(unpacked, payload);
}

// ---------- empty payload ----------

TEST_F(FramerTest, EmptyPayload) {
    std::vector<uint8_t> payload;
    auto packed = framer->pack(payload);

    ASSERT_EQ(packed.size(), 4u);
    EXPECT_EQ(packed[0], 0x00);
    EXPECT_EQ(packed[1], 0x00);
    EXPECT_EQ(packed[2], 0x00);
    EXPECT_EQ(packed[3], 0x00);

    auto unpacked = framer->unpack(packed);
    EXPECT_TRUE(unpacked.empty());
}

// ---------- large payload ----------

TEST_F(FramerTest, LargePayload) {
    std::vector<uint8_t> payload(100'000, 0xAB);
    auto packed = framer->pack(payload);
    EXPECT_EQ(packed.size(), 4 + payload.size());

    auto unpacked = framer->unpack(packed);
    EXPECT_EQ(unpacked, payload);
}

// ---------- header encoding ----------

TEST_F(FramerTest, HeaderIsBigEndian) {
    std::vector<uint8_t> payload(256, 0x00);
    auto packed = framer->pack(payload);

    EXPECT_EQ(packed[0], 0x00);
    EXPECT_EQ(packed[1], 0x00);
    EXPECT_EQ(packed[2], 0x01);
    EXPECT_EQ(packed[3], 0x00);
}

// ---------- error cases ----------

TEST_F(FramerTest, UnpackTooShortForHeader) {
    std::vector<uint8_t> stream = {0x00, 0x00};
    EXPECT_THROW(framer->unpack(stream), std::runtime_error);
}

TEST_F(FramerTest, UnpackTooShortForPayload) {
    std::vector<uint8_t> stream = {0x00, 0x00, 0x00, 0x0A, 0x01, 0x02};
    EXPECT_THROW(framer->unpack(stream), std::runtime_error);
}
