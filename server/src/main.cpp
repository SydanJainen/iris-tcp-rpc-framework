#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <nlohmann/json.hpp>

#include "container.h"
#include "request_handler.h"

#include "adapters/length_prefixed_framer.h"
#include "adapters/json_serializer.h"
#include "adapters/dispatcher.h"
#include "adapters/metrics_collector.h"
#include "adapters/in_memory_transaction_log.h"

#include "domain/add_func.h"
#include "domain/reverse_func.h"
#include "domain/multiply_func.h"
#include "domain/fibonacci_func.h"
#include "domain/to_base_func.h"
#include "domain/tfidf_func.h"

#include "adapters/file_corpus.h"

#ifdef _WIN32
#include "adapters/win/win_tcp_listener.h"
#else
#include "adapters/linux/linux_tcp_listener.h"
#endif

static uint16_t parse_port(int argc, char* argv[]) {
    uint16_t port = 5555;
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "--port") {
            port = static_cast<uint16_t>(std::stoi(argv[i + 1]));
            break;
        }
    }
    return port;
}

static nlohmann::json load_api_spec(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Warning: could not open " << path << ", using empty spec." << std::endl;
        return nlohmann::json::object();
    }
    return nlohmann::json::parse(file);
}

int main(int argc, char* argv[]) {
    uint16_t port = parse_port(argc, argv);

    // Load API spec
    nlohmann::json api_spec = load_api_spec("api_spec.json");

    //Composition Root
    iris::Container container;

    // Register framer
    container.register_singleton<std::shared_ptr<iris::IFramer>>(
        "framer", []() -> std::shared_ptr<iris::IFramer> {
            return std::make_shared<iris::LengthPrefixedFramer>();
        });

    // Register serializer
    container.register_singleton<std::shared_ptr<iris::ISerializer>>(
        "serializer", []() -> std::shared_ptr<iris::ISerializer> {
            return std::make_shared<iris::JsonSerializer>();
        });

    // Register metrics
    container.register_singleton<std::shared_ptr<iris::IMetrics>>(
        "metrics", []() -> std::shared_ptr<iris::IMetrics> {
            return std::make_shared<iris::MetricsCollector>();
        });

    // Register transaction log
    container.register_singleton<std::shared_ptr<iris::ITransactionLog>>(
        "transaction_log", []() -> std::shared_ptr<iris::ITransactionLog> {
            return std::make_shared<iris::InMemoryTransactionLog>();
        });

    // Register corpus
    container.register_singleton<std::shared_ptr<iris::ICorpus>>(
        "corpus", []() -> std::shared_ptr<iris::ICorpus> {
            return std::make_shared<iris::FileCorpus>("server/data/");
        });

    // Resolve corpus for injection into TfidfFunc
    auto& corpus = container.resolve<std::shared_ptr<iris::ICorpus>>("corpus");

    // Register dispatcher with API functions
    container.register_singleton<std::shared_ptr<iris::IDispatcher>>(
        "dispatcher", [&corpus]() -> std::shared_ptr<iris::IDispatcher> {
            auto dispatcher = std::make_shared<iris::Dispatcher>();
            dispatcher->register_function(std::make_unique<iris::AddFunc>());
            dispatcher->register_function(std::make_unique<iris::ReverseFunc>());
            dispatcher->register_function(std::make_unique<iris::MultiplyFunc>());
            dispatcher->register_function(std::make_unique<iris::FibonacciFunc>());
            dispatcher->register_function(std::make_unique<iris::ToBaseFunc>());
            dispatcher->register_function(std::make_unique<iris::TfidfFunc>(*corpus));
            return dispatcher;
        });

    // Register listener — platform-specific factory selected at compile time
    container.register_singleton<std::shared_ptr<iris::IListener>>(
        "listener", []() -> std::shared_ptr<iris::IListener> {
#ifdef _WIN32
            return std::make_shared<iris::WinTcpListener>();
#else
            return std::make_shared<iris::LinuxTcpListener>();
#endif
        });

    // Resolve dependencies
    auto& framer = container.resolve<std::shared_ptr<iris::IFramer>>("framer");
    auto& serializer = container.resolve<std::shared_ptr<iris::ISerializer>>("serializer");
    auto& dispatcher = container.resolve<std::shared_ptr<iris::IDispatcher>>("dispatcher");
    auto& metrics = container.resolve<std::shared_ptr<iris::IMetrics>>("metrics");
    auto& transaction_log = container.resolve<std::shared_ptr<iris::ITransactionLog>>("transaction_log");
    auto& listener = container.resolve<std::shared_ptr<iris::IListener>>("listener");

    // Build RequestHandler
    iris::RequestHandler handler(*framer, *serializer, *dispatcher, *metrics, *transaction_log, api_spec);

    // Start listener
    try {
        listener->start(port);
        std::cout << "Iris server listening on port " << port << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start listener: " << e.what() << std::endl;
        return 1;
    }

    while (true) {
        try {
            auto conn = listener->accept();
            std::cout << "Client connected." << std::endl;
            handler.handle_connection(*conn);
            std::cout << "Client disconnected." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Accept error: " << e.what() << std::endl;
        }
    }

    listener->stop();
    return 0;
}
