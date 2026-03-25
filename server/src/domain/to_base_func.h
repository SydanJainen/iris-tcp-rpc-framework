#ifndef IRIS_TO_BASE_FUNC_H
#define IRIS_TO_BASE_FUNC_H

#include <algorithm>
#include <any>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#include "ports/i_api_function.h"

namespace iris {

class ToBaseFunc : public IApiFunction {
public:
    std::string name() const override { return "to_base"; }

    std::any execute(const std::vector<std::any>& args) override {
        if (args.size() != 2) {
            throw std::invalid_argument( "BAD_ARGUMENTS: to_base expects 2 arguments, got " + std::to_string(args.size()));
        }

        int number, base;
        try {
            number = std::any_cast<int>(args[0]);
            base = std::any_cast<int>(args[1]);
        } catch (const std::bad_any_cast&) {
            throw std::invalid_argument( "BAD_ARGUMENTS: to_base expects two int arguments");
        }

        if (base < 2 || base > 36) {
            throw std::invalid_argument( "BAD_ARGUMENTS: base must be between 2 and 36");
        }

        if (number == 0) {
            return std::string("0");
        }

        bool negative = number < 0;

        long long abs_number = negative ? -static_cast<long long>(number) : static_cast<long long>(number);

        static constexpr char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::string result;

        while (abs_number > 0) {
            result += digits[abs_number % base];
            abs_number /= base;
        }

        if (negative) {
            result += '-';
        }

        std::reverse(result.begin(), result.end());
        return result;
    }
};

}

#endif
