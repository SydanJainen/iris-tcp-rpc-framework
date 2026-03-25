#ifndef IRIS_FIBONACCI_FUNC_H
#define IRIS_FIBONACCI_FUNC_H

#include <any>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "ports/i_api_function.h"

namespace iris {

class FibonacciFunc : public IApiFunction {

public:
    std::string name() const override { return "fibonacci"; }

    std::any execute(const std::vector<std::any>& args) override {
        if (args.size() != 1) {
            throw std::invalid_argument("BAD_ARGUMENTS: fibonacci expects 1 argument, got " + std::to_string(args.size()));
        }
        int n;
        try {
            n = std::any_cast<int>(args[0]);
        } catch (const std::bad_any_cast&) {
            throw std::invalid_argument("BAD_ARGUMENTS: fibonacci expects an int argument");
        }

        if (n < 0) {
            throw std::invalid_argument( "BAD_ARGUMENTS: n must be non-negative");
        }
        if (n > 93) {
            throw std::invalid_argument( "BAD_ARGUMENTS: n must be <= 93 to fit in int64");
        }

        return static_cast<long long>(fibonacci(n));
    }

private:
    static int64_t fibonacci(int n) {
        if (n == 0) return 0;
        if (n == 1) return 1;

        int64_t prev = 0;
        int64_t curr = 1;

        for (int i = 2; i <= n; ++i) {
            int64_t next = prev + curr;
            prev = curr;
            curr = next;
        }

        return curr;
    }
};

}

#endif
