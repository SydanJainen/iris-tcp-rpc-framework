#ifndef IRIS_REQUEST_HANDLER_H
#define IRIS_REQUEST_HANDLER_H

#include <any>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "connection.h"
#include "ports/i_dispatcher.h"
#include "ports/i_framer.h"
#include "ports/i_serializer.h"

namespace iris {

class RequestHandler {
public:
    RequestHandler(IFramer& framer,
                   ISerializer& serializer,
                   IDispatcher& dispatcher,
                   nlohmann::json api_spec);

    void handle_connection(Connection& conn);

private:
    IFramer& framer_;
    ISerializer& serializer_;
    IDispatcher& dispatcher_;
    nlohmann::json api_spec_;

    std::vector<uint8_t> read_exact(Connection& conn, size_t n);
    void send_json_response(Connection& conn, const nlohmann::json& response);
    void send_error_response(Connection& conn,
                             const std::string& id,
                             const std::string& error_code,
                             const std::string& message);

    static std::any json_to_any(const nlohmann::json& val);
    static nlohmann::json any_to_json(const std::any& val);
};

}

#endif
