#ifndef IRIS_MULTIPLY_FUNC_H
#define IRIS_MULTIPLY_FUNC_H

#include <any>
#include <stdexcept>
#include <string>
#include <vector>

#include "ports/i_api_function.h"

namespace iris {

class MultiplyFunc : public IApiFunction {
public:
    std::string name() const override { return "multiply"; }

    std::any execute(const std::vector<std::any>& args) override {
        if (args.size() != 2) {
            throw std::invalid_argument(
                "BAD_ARGUMENTS: multiply expects 2 arguments, got " +
                std::to_string(args.size()));
        }
        try {
            double a = std::any_cast<double>(args[0]);
            double b = std::any_cast<double>(args[1]);
            return a * b;
        } catch (const std::bad_any_cast&) {
            throw std::invalid_argument(
                "BAD_ARGUMENTS: multiply expects two double arguments");
        }
    }
};

}

#endif
