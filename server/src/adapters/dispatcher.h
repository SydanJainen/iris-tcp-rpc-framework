#ifndef IRIS_DISPATCHER_H
#define IRIS_DISPATCHER_H

#include <any>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ports/i_dispatcher.h"

namespace iris {

class Dispatcher : public IDispatcher {
public:
    void register_function(std::unique_ptr<IApiFunction> func) override;
    std::any dispatch(const std::string& command,
                      const std::vector<std::any>& args) override;

private:
    std::map<std::string, std::unique_ptr<IApiFunction>> functions_;
};

}

#endif
