#ifndef IRIS_IN_MEMORY_TRANSACTION_LOG_H
#define IRIS_IN_MEMORY_TRANSACTION_LOG_H

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "ports/i_transaction_log.h"
#include "types.h"

namespace iris {

class InMemoryTransactionLog : public ITransactionLog {
public:
    InMemoryTransactionLog() = default;

    void log(const TransactionRecord& record) override {
        records_.push_back(record);
    }

    std::vector<TransactionRecord> get_history(int limit = 20) override {
        if (limit <= 0 || records_.empty()) {
            return {};
        }
        int total = static_cast<int>(records_.size());
        int start = std::max(0, total - limit);
        return std::vector<TransactionRecord>(
            records_.begin() + start,
            records_.end()
        );
    }

    std::optional<TransactionRecord> get_by_id(const std::string& uuid) override {
        for (const auto& rec : records_) {
            if (rec.id == uuid) {
                return rec;
            }
        }
        return std::nullopt;
    }

private:
    std::vector<TransactionRecord> records_;
};

}

#endif
