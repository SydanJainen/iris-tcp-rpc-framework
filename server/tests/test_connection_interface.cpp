#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "ports/i_connection.h"

namespace {

class MockConnection : public iris::IConnection {
public:
    MockConnection() = default;
    explicit MockConnection(bool valid) : valid_(valid) {}

    void send(const std::vector<uint8_t>& data) override {
        send_buffer_.insert(send_buffer_.end(), data.begin(), data.end());
    }

    std::vector<uint8_t> recv(size_t max_bytes) override {
        if (closed_) {
            return {};
        }
        size_t to_read = std::min(max_bytes, recv_buffer_.size());
        std::vector<uint8_t> result(recv_buffer_.begin(),
                                     recv_buffer_.begin() + static_cast<ptrdiff_t>(to_read));
        recv_buffer_.erase(recv_buffer_.begin(),
                           recv_buffer_.begin() + static_cast<ptrdiff_t>(to_read));
        return result;
    }

    bool is_valid() const override {
        return valid_;
    }

    // Test helpers
    void set_recv_data(const std::vector<uint8_t>& data) {
        recv_buffer_ = data;
    }

    void set_closed(bool closed) {
        closed_ = closed;
    }

    void set_valid(bool valid) {
        valid_ = valid;
    }

    const std::vector<uint8_t>& get_send_buffer() const {
        return send_buffer_;
    }

private:
    std::vector<uint8_t> send_buffer_;
    std::vector<uint8_t> recv_buffer_;
    bool valid_ = true;
    bool closed_ = false;
};

TEST(ConnectionInterfaceTest, SendDataWritesToBuffer) {
    MockConnection conn;
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    conn.send(data);

    EXPECT_EQ(conn.get_send_buffer(), data);
}

TEST(ConnectionInterfaceTest, SendMultipleChunks) {
    MockConnection conn;
    conn.send({0x01, 0x02});
    conn.send({0x03, 0x04});

    std::vector<uint8_t> expected = {0x01, 0x02, 0x03, 0x04};
    EXPECT_EQ(conn.get_send_buffer(), expected);
}

TEST(ConnectionInterfaceTest, RecvReturnsDataFromBuffer) {
    MockConnection conn;
    std::vector<uint8_t> data = {0xAA, 0xBB, 0xCC};
    conn.set_recv_data(data);

    auto received = conn.recv(3);
    EXPECT_EQ(received, data);
}

TEST(ConnectionInterfaceTest, RecvReturnsPartialData) {
    MockConnection conn;
    conn.set_recv_data({0x01, 0x02, 0x03, 0x04});

    auto first = conn.recv(2);
    EXPECT_EQ(first.size(), 2u);
    EXPECT_EQ(first[0], 0x01);
    EXPECT_EQ(first[1], 0x02);

    auto second = conn.recv(2);
    EXPECT_EQ(second.size(), 2u);
    EXPECT_EQ(second[0], 0x03);
    EXPECT_EQ(second[1], 0x04);
}

TEST(ConnectionInterfaceTest, RecvReturnsEmptyOnClosedConnection) {
    MockConnection conn;
    conn.set_recv_data({0x01, 0x02});
    conn.set_closed(true);

    auto received = conn.recv(10);
    EXPECT_TRUE(received.empty());
}

TEST(ConnectionInterfaceTest, IsValidReturnsTrue) {
    MockConnection conn(true);
    EXPECT_TRUE(conn.is_valid());
}

TEST(ConnectionInterfaceTest, IsValidReturnsFalse) {
    MockConnection conn(false);
    EXPECT_FALSE(conn.is_valid());
}

TEST(ConnectionInterfaceTest, IsValidStateCanChange) {
    MockConnection conn(true);
    EXPECT_TRUE(conn.is_valid());

    conn.set_valid(false);
    EXPECT_FALSE(conn.is_valid());
}

TEST(ConnectionInterfaceTest, PolymorphicAccess) {
    MockConnection mock;
    mock.set_recv_data({0x10, 0x20});

    iris::IConnection& iface = mock;
    iface.send({0xFF});
    auto data = iface.recv(2);

    EXPECT_EQ(mock.get_send_buffer(), std::vector<uint8_t>{0xFF});
    EXPECT_EQ(data, (std::vector<uint8_t>{0x10, 0x20}));
    EXPECT_TRUE(iface.is_valid());
}

}
