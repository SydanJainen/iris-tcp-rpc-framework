#ifndef IRIS_I_API_FUNCTION_H
#define IRIS_I_API_FUNCTION_H

#include <any>
#include <string>
#include <vector>

namespace iris {

class IApiFunction {
public:
    virtual ~IApiFunction() = default;

    virtual std::string name() const = 0;
    virtual std::any execute(const std::vector<std::any>& args) = 0;
};

}

#endif
