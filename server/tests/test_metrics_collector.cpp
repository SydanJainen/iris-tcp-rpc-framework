#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "adapters/metrics_collector.h"
#include "types.h"

// ---- Empty state ----

TEST(MetricsCollectorTest, EmptyReturnsEmptySummary) {
    iris::MetricsCollector mc;
    auto summary = mc.get_summary();
    EXPECT_TRUE(summary.empty());
}

// ---- Single command ----

TEST(MetricsCollectorTest, SingleSuccessRecord) {
    iris::MetricsCollector mc;
    mc.record("add", 10.0, true);

    auto summary = mc.get_summary();
    ASSERT_EQ(summary.size(), 1u);
    EXPECT_EQ(summary[0].command, "add");
    EXPECT_EQ(summary[0].total_calls, 1);
    EXPECT_DOUBLE_EQ(summary[0].avg_time_ms, 10.0);
    EXPECT_EQ(summary[0].total_errors, 0);
}

TEST(MetricsCollectorTest, SingleFailureRecord) {
    iris::MetricsCollector mc;
    mc.record("add", 5.0, false);

    auto summary = mc.get_summary();
    ASSERT_EQ(summary.size(), 1u);
    EXPECT_EQ(summary[0].total_calls, 1);
    EXPECT_EQ(summary[0].total_errors, 1);
}

TEST(MetricsCollectorTest, MultipleRecordsAverage) {
    iris::MetricsCollector mc;
    mc.record("add", 10.0, true);
    mc.record("add", 20.0, true);
    mc.record("add", 30.0, true);

    auto summary = mc.get_summary();
    ASSERT_EQ(summary.size(), 1u);
    EXPECT_EQ(summary[0].total_calls, 3);
    EXPECT_DOUBLE_EQ(summary[0].avg_time_ms, 20.0);
    EXPECT_EQ(summary[0].total_errors, 0);
}

TEST(MetricsCollectorTest, MixedSuccessAndError) {
    iris::MetricsCollector mc;
    mc.record("reverse", 5.0, true);
    mc.record("reverse", 10.0, false);
    mc.record("reverse", 15.0, true);

    auto summary = mc.get_summary();
    ASSERT_EQ(summary.size(), 1u);
    EXPECT_EQ(summary[0].total_calls, 3);
    EXPECT_EQ(summary[0].total_errors, 1);
    EXPECT_DOUBLE_EQ(summary[0].avg_time_ms, 10.0);
}

// ---- Multiple commands ----

TEST(MetricsCollectorTest, SeparateCommandsSeparateEntries) {
    iris::MetricsCollector mc;
    mc.record("add", 10.0, true);
    mc.record("reverse", 20.0, true);

    auto summary = mc.get_summary();
    EXPECT_EQ(summary.size(), 2u);
}

TEST(MetricsCollectorTest, PerCommandAveragesAreIndependent) {
    iris::MetricsCollector mc;
    mc.record("add", 10.0, true);
    mc.record("add", 30.0, true);
    mc.record("reverse", 100.0, true);

    // Build map
    std::map<std::string, iris::MetricsSummary> by_cmd;
    for (const auto& s : mc.get_summary()) {
        by_cmd[s.command] = s;
    }

    EXPECT_DOUBLE_EQ(by_cmd["add"].avg_time_ms, 20.0);
    EXPECT_DOUBLE_EQ(by_cmd["reverse"].avg_time_ms, 100.0);
}

TEST(MetricsCollectorTest, PerCommandErrorCountsAreIndependent) {
    iris::MetricsCollector mc;
    mc.record("add", 1.0, false);
    mc.record("add", 1.0, false);
    mc.record("reverse", 1.0, true);

    std::map<std::string, iris::MetricsSummary> by_cmd;
    for (const auto& s : mc.get_summary()) {
        by_cmd[s.command] = s;
    }

    EXPECT_EQ(by_cmd["add"].total_errors, 2);
    EXPECT_EQ(by_cmd["reverse"].total_errors, 0);
}

// ---- Edge cases ----

TEST(MetricsCollectorTest, ZeroDuration) {
    iris::MetricsCollector mc;
    mc.record("add", 0.0, true);

    auto summary = mc.get_summary();
    ASSERT_EQ(summary.size(), 1u);
    EXPECT_DOUBLE_EQ(summary[0].avg_time_ms, 0.0);
}

TEST(MetricsCollectorTest, LargeNumberOfRecords) {
    iris::MetricsCollector mc;
    for (int i = 0; i < 1000; ++i) {
        mc.record("fib", static_cast<double>(i), i % 2 == 0);
    }

    auto summary = mc.get_summary();
    ASSERT_EQ(summary.size(), 1u);
    EXPECT_EQ(summary[0].total_calls, 1000);
    EXPECT_EQ(summary[0].total_errors, 500);
}
