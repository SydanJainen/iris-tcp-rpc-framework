#ifndef IRIS_REVERSE_FUNC_H
#define IRIS_REVERSE_FUNC_H

#include <algorithm>
#include <any>
#include <stdexcept>
#include <string>
#include <vector>

#include "ports/i_api_function.h"

namespace iris {

class ReverseFunc : public IApiFunction {
public:
    std::string name() const override { return "reverse"; }

    std::any execute(const std::vector<std::any>& args) override {
        if (args.size() != 1) {
            throw std::invalid_argument(
                "BAD_ARGUMENTS: reverse expects 1 argument, got " +
                std::to_string(args.size()));
        }
        try {
            std::string s = std::any_cast<std::string>(args[0]);
            std::reverse(s.begin(), s.end());
            return s;
        } catch (const std::bad_any_cast&) {
            throw std::invalid_argument(
                "BAD_ARGUMENTS: reverse expects a string argument");
        }
    }
};

}

#endif
