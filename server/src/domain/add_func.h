#ifndef IRIS_ADD_FUNC_H
#define IRIS_ADD_FUNC_H

#include <any>
#include <stdexcept>
#include <string>
#include <vector>
#include "ports/i_api_function.h"

namespace iris {

class AddFunc : public IApiFunction {

public:
    std::string name() const override { return "add"; }
    std::any execute(const std::vector<std::any>& args) override {
        if (args.size() != 2) {
            throw std::invalid_argument( "BAD_ARGUMENTS: add expects 2 arguments, got " + std::to_string(args.size()));
        }
        try {

            int a = std::any_cast<int>(args[0]);
            int b = std::any_cast<int>(args[1]);
            return a + b;

        } catch (const std::bad_any_cast&) {
            throw std::invalid_argument( "BAD_ARGUMENTS: add expects two int arguments");
        }
    }
};

}

#endif
