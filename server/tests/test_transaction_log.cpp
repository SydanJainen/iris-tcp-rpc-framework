#include <gtest/gtest.h>

#include <any>
#include <optional>
#include <string>
#include <vector>

#include "adapters/in_memory_transaction_log.h"
#include "types.h"

static iris::TransactionRecord make_record(const std::string& cmd = "add",
                                            const std::string& status = "ok") {
    iris::TransactionRecord rec;
    rec.id = "uuid-" + cmd + "-" + status;
    rec.timestamp = "2026-01-01T00:00:00Z";
    rec.command = cmd;
    rec.args = {};
    rec.status = status;
    if (status == "ok") {
        rec.result = 42;
    } else {
        rec.error_code = "INTERNAL_ERROR";
        rec.message = "something went wrong";
    }
    rec.processing_time_ms = 1.0;
    return rec;
}

// ---- Empty state ----

TEST(InMemoryTransactionLogTest, EmptyGetHistoryReturnsEmpty) {
    iris::InMemoryTransactionLog log;
    auto history = log.get_history(10);
    EXPECT_TRUE(history.empty());
}

TEST(InMemoryTransactionLogTest, EmptyGetByIdReturnsNullopt) {
    iris::InMemoryTransactionLog log;
    auto result = log.get_by_id("nonexistent-uuid");
    EXPECT_FALSE(result.has_value());
}

// ---- Log and retrieve ----

TEST(InMemoryTransactionLogTest, LogSingleRecord) {
    iris::InMemoryTransactionLog log;
    auto rec = make_record("add", "ok");
    log.log(rec);

    auto history = log.get_history(10);
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0].id, rec.id);
    EXPECT_EQ(history[0].command, "add");
}

TEST(InMemoryTransactionLogTest, LogMultipleRecordsInOrder) {
    iris::InMemoryTransactionLog log;
    std::vector<iris::TransactionRecord> records;
    for (int i = 0; i < 5; ++i) {
        auto r = make_record("cmd_" + std::to_string(i), "ok");
        r.id = "id-" + std::to_string(i);
        records.push_back(r);
        log.log(r);
    }

    auto history = log.get_history(10);
    ASSERT_EQ(history.size(), 5u);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(history[i].id, records[i].id);
    }
}

// ---- get_history with limit ----

TEST(InMemoryTransactionLogTest, GetHistoryDefaultLimitTwenty) {
    iris::InMemoryTransactionLog log;
    for (int i = 0; i < 25; ++i) {
        auto r = make_record("add", "ok");
        r.id = "id-" + std::to_string(i);
        log.log(r);
    }

    auto history = log.get_history();
    EXPECT_EQ(history.size(), 20u);
}

TEST(InMemoryTransactionLogTest, GetHistoryWithLimit) {
    iris::InMemoryTransactionLog log;
    for (int i = 0; i < 10; ++i) {
        auto r = make_record("add", "ok");
        r.id = "id-" + std::to_string(i);
        log.log(r);
    }

    auto history = log.get_history(3);
    EXPECT_EQ(history.size(), 3u);
}

TEST(InMemoryTransactionLogTest, GetHistoryReturnsLastN) {
    iris::InMemoryTransactionLog log;
    std::vector<iris::TransactionRecord> records;
    for (int i = 0; i < 5; ++i) {
        auto r = make_record("add", "ok");
        r.id = "id-" + std::to_string(i);
        records.push_back(r);
        log.log(r);
    }

    auto history = log.get_history(2);
    ASSERT_EQ(history.size(), 2u);
    EXPECT_EQ(history[0].id, "id-3");
    EXPECT_EQ(history[1].id, "id-4");
}

TEST(InMemoryTransactionLogTest, GetHistoryLimitLargerThanStored) {
    iris::InMemoryTransactionLog log;
    for (int i = 0; i < 3; ++i) {
        auto r = make_record("add", "ok");
        r.id = "id-" + std::to_string(i);
        log.log(r);
    }

    auto history = log.get_history(100);
    EXPECT_EQ(history.size(), 3u);
}

TEST(InMemoryTransactionLogTest, GetHistoryZeroLimitReturnsEmpty) {
    iris::InMemoryTransactionLog log;
    log.log(make_record());

    auto history = log.get_history(0);
    EXPECT_TRUE(history.empty());
}

TEST(InMemoryTransactionLogTest, GetHistoryNegativeLimitReturnsEmpty) {
    iris::InMemoryTransactionLog log;
    log.log(make_record());

    auto history = log.get_history(-1);
    EXPECT_TRUE(history.empty());
}

// ---- get_by_id ----

TEST(InMemoryTransactionLogTest, GetByIdFindsRecord) {
    iris::InMemoryTransactionLog log;
    auto rec = make_record("add", "ok");
    rec.id = "specific-uuid-1234";
    log.log(rec);

    auto found = log.get_by_id("specific-uuid-1234");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, "specific-uuid-1234");
    EXPECT_EQ(found->command, "add");
}

TEST(InMemoryTransactionLogTest, GetByIdReturnsNulloptForUnknown) {
    iris::InMemoryTransactionLog log;
    log.log(make_record());

    auto result = log.get_by_id("nonexistent-uuid");
    EXPECT_FALSE(result.has_value());
}

TEST(InMemoryTransactionLogTest, GetByIdFindsCorrectAmongMany) {
    iris::InMemoryTransactionLog log;
    for (int i = 0; i < 10; ++i) {
        auto r = make_record("cmd_" + std::to_string(i), "ok");
        r.id = "id-" + std::to_string(i);
        log.log(r);
    }

    auto found = log.get_by_id("id-5");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->command, "cmd_5");
}

TEST(InMemoryTransactionLogTest, GetByIdAllUniqueIds) {
    iris::InMemoryTransactionLog log;
    std::vector<iris::TransactionRecord> records;
    for (int i = 0; i < 5; ++i) {
        auto r = make_record("add", "ok");
        r.id = "uuid-" + std::to_string(i);
        records.push_back(r);
        log.log(r);
    }

    for (const auto& rec : records) {
        auto found = log.get_by_id(rec.id);
        ASSERT_TRUE(found.has_value());
        EXPECT_EQ(found->id, rec.id);
    }
}
