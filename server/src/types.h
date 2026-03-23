#ifndef IRIS_TYPES_H
#define IRIS_TYPES_H

#include <any>
#include <string>
#include <vector>

namespace iris {

struct TransactionRecord {
    std::string id;
    std::string timestamp;
    std::string command;
    std::vector<std::any> args;
    std::string status;
    std::any result;
    std::string error_code;
    std::string message;
    double processing_time_ms = 0.0;
};

struct MetricsSummary {
    std::string command;
    int total_calls = 0;
    double avg_time_ms = 0.0;
    int total_errors = 0;
};

}

#endif
