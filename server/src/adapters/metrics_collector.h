#ifndef IRIS_METRICS_COLLECTOR_H
#define IRIS_METRICS_COLLECTOR_H

#include <map>
#include <string>
#include <vector>

#include "ports/i_metrics.h"
#include "types.h"

namespace iris {

class MetricsCollector : public IMetrics {
public:
    MetricsCollector() = default;

    void record(const std::string& cmd,
                double duration_ms,
                bool success) override {
        auto& entry = data_[cmd];
        entry.total_calls += 1;
        entry.total_ms += duration_ms;
        if (!success) {
            entry.total_errors += 1;
        }
    }

    std::vector<MetricsSummary> get_summary() override {
        std::vector<MetricsSummary> result;
        result.reserve(data_.size());
        for (const auto& [cmd, entry] : data_) {
            MetricsSummary s;
            s.command = cmd;
            s.total_calls = entry.total_calls;
            s.avg_time_ms = (entry.total_calls > 0)
                ? entry.total_ms / entry.total_calls
                : 0.0;
            s.total_errors = entry.total_errors;
            result.push_back(s);
        }
        return result;
    }

private:
    struct Entry {
        int total_calls = 0;
        double total_ms = 0.0;
        int total_errors = 0;
    };

    std::map<std::string, Entry> data_;
};

}

#endif
