#ifndef IRIS_I_TRANSACTION_LOG_H
#define IRIS_I_TRANSACTION_LOG_H

#include <optional>
#include <string>
#include <vector>

#include "types.h"

namespace iris {

class ITransactionLog {
public:
    virtual ~ITransactionLog() = default;

    virtual void log(const TransactionRecord& record) = 0;

    virtual std::vector<TransactionRecord> get_history(int limit = 20) = 0;

    virtual std::optional<TransactionRecord> get_by_id(const std::string& uuid) = 0;
};

}

#endif
