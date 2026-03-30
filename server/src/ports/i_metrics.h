#ifndef IRIS_I_METRICS_H
#define IRIS_I_METRICS_H

#include <string>
#include <vector>

#include "types.h"

namespace iris {

class IMetrics {
public:
    virtual ~IMetrics() = default;

    virtual void record(const std::string& cmd, double duration_ms, bool success) = 0;

    virtual std::vector<MetricsSummary> get_summary() = 0;
};

}

#endif
