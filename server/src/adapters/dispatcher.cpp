#include "dispatcher.h"

#include <stdexcept>

namespace iris {

void Dispatcher::register_function(std::unique_ptr<IApiFunction> func) {
    std::string func_name = func->name();
    functions_[func_name] = std::move(func);
}

std::any Dispatcher::dispatch(const std::string& command,
                              const std::vector<std::any>& args) {
    auto it = functions_.find(command);
    if (it == functions_.end()) {
        throw std::runtime_error("UNKNOWN_COMMAND: " + command);
    }
    try {
        return it->second->execute(args);
    } catch (const std::invalid_argument&) {
        throw;
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("INTERNAL_ERROR: ") + e.what());
    }
}

}
