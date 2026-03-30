#include "request_handler.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace iris {

static std::string current_iso8601() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_val{};
#ifdef _WIN32
    gmtime_s(&tm_val, &t);
#else
    gmtime_r(&t, &tm_val);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

RequestHandler::RequestHandler(IFramer& framer,
                               ISerializer& serializer,
                               IDispatcher& dispatcher,
                               IMetrics& metrics,
                               ITransactionLog& transaction_log,
                               nlohmann::json api_spec):
    framer_(framer),
    serializer_(serializer),
    dispatcher_(dispatcher),
    metrics_(metrics),
    transaction_log_(transaction_log),
    api_spec_(std::move(api_spec)) {}

void RequestHandler::handle_connection(IConnection& conn) {
    while (true) {
        try {
            std::vector<uint8_t> header = read_exact(conn, 4);
            if (header.empty()) {
                break;
            }

            uint32_t payload_len =
                (static_cast<uint32_t>(header[0]) << 24) |
                (static_cast<uint32_t>(header[1]) << 16) |
                (static_cast<uint32_t>(header[2]) << 8)  |
                (static_cast<uint32_t>(header[3]));

            std::vector<uint8_t> payload = read_exact(conn, payload_len);
            if (payload.size() < payload_len) {
                break;
            }

            nlohmann::json request;
            try {
                request = serializer_.deserialize(payload);
            } catch (const std::exception&) {
                send_error_response(conn, "", "PARSE_ERROR", "Failed to parse JSON");
                continue;
            }

            std::string id = request.value("id", "");
            std::string command = request.value("cmd", "");

            if (command == "get_spec") {
                nlohmann::json response;
                response["id"] = id;
                response["status"] = "ok";
                response["result"] = api_spec_;
                send_json_response(conn, response);
                continue;
            }

            if (command.empty()) {
                send_error_response(conn, id, "BAD_ARGUMENTS", "Missing 'cmd' field");
                continue;
            }

            std::vector<std::any> args;
            if (request.contains("args") && request["args"].is_array()) {
                for (const auto& arg : request["args"]) {
                    args.push_back(json_to_any(arg));
                }
            }

            {
                TransactionRecord rec;
                rec.id = id;
                rec.timestamp = current_iso8601();
                rec.command = command;
                rec.args = args;

                auto t_start = std::chrono::steady_clock::now();
                bool success = false;
                std::string err_code;
                std::string err_msg;
                std::any dispatch_result;

                try {
                    dispatch_result = dispatcher_.dispatch(command, args);
                    success = true;
                } catch (const std::runtime_error& e) {
                    std::string msg = e.what();
                    if (msg.starts_with("UNKNOWN_COMMAND")) {
                        err_code = "UNKNOWN_COMMAND";
                    } else {
                        err_code = "INTERNAL_ERROR";
                    }
                    err_msg = msg;
                } catch (const std::invalid_argument& e) {
                    err_code = "BAD_ARGUMENTS";
                    err_msg = e.what();
                } catch (const std::exception& e) {
                    err_code = "INTERNAL_ERROR";
                    err_msg = e.what();
                }

                auto t_end = std::chrono::steady_clock::now();
                double duration_ms = std::chrono::duration<double, std::milli>(
                    t_end - t_start).count();

                // Record metrics
                metrics_.record(command, duration_ms, success);

                // Build and log transaction record
                rec.processing_time_ms = duration_ms;
                if (success) {
                    rec.status = "ok";
                    rec.result = dispatch_result;
                } else {
                    rec.status = "error";
                    rec.error_code = err_code;
                    rec.message = err_msg;
                }
                transaction_log_.log(rec);

                // Send response
                if (success) {
                    nlohmann::json response;
                    response["id"] = id;
                    response["status"] = "ok";
                    response["result"] = any_to_json(dispatch_result);
                    send_json_response(conn, response);
                } else {
                    send_error_response(conn, id, err_code, err_msg);
                }
            }

        } catch (const std::exception& e) {
            std::cerr << "RequestHandler: connection error: "
                      << e.what() << std::endl;
            break;
        }
    }
}

std::vector<uint8_t> RequestHandler::read_exact(IConnection& conn, size_t n) {
    std::vector<uint8_t> buffer;
    buffer.reserve(n);
    while (buffer.size() < n) {
        auto chunk = conn.recv(n - buffer.size());
        if (chunk.empty()) {
            if (buffer.empty()) return {};
            break;
        }
        buffer.insert(buffer.end(), chunk.begin(), chunk.end());
    }
    return buffer;
}

void RequestHandler::send_json_response(IConnection& conn,
                                         const nlohmann::json& response) {
    auto payload = serializer_.serialize(response);
    auto frame = framer_.pack(payload);
    conn.send(frame);
}

void RequestHandler::send_error_response(IConnection& conn,
                                          const std::string& id,
                                          const std::string& error_code,
                                          const std::string& message) {
    nlohmann::json response;
    response["id"] = id;
    response["status"] = "error";
    response["error_code"] = error_code;
    response["message"] = message;
    send_json_response(conn, response);
}

std::any RequestHandler::json_to_any(const nlohmann::json& val) {
    if (val.is_number_integer()) {
        return val.get<int>();
    }
    if (val.is_number_float()) {
        return val.get<double>();
    }
    if (val.is_string()) {
        return val.get<std::string>();
    }
    if (val.is_boolean()) {
        return val.get<bool>();
    }
    return val.dump();
}

nlohmann::json RequestHandler::any_to_json(const std::any& val) {
    if (val.type() == typeid(int)) {
        return std::any_cast<int>(val);
    }
    if (val.type() == typeid(double)) {
        return std::any_cast<double>(val);
    }
    if (val.type() == typeid(std::string)) {
        return std::any_cast<std::string>(val);
    }
    if (val.type() == typeid(bool)) {
        return std::any_cast<bool>(val);
    }
    if (val.type() == typeid(long long)) {
        return std::any_cast<long long>(val);
    }
    return nullptr;
}

}
