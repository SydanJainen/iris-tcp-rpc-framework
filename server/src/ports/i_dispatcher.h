#ifndef IRIS_I_DISPATCHER_H
#define IRIS_I_DISPATCHER_H

#include <any>
#include <memory>
#include <string>
#include <vector>

#include "i_api_function.h"

namespace iris {

class IDispatcher {
public:
    virtual ~IDispatcher() = default;

    virtual void register_function(std::unique_ptr<IApiFunction> func) = 0;
    virtual std::any dispatch(const std::string& command, const std::vector<std::any>& args) = 0;
};

}

#endif
