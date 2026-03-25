#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "ports/i_api_function.h"
#include "adapters/dispatcher.h"

class MockFunc : public iris::IApiFunction {
public:
    std::string name() const override { return "mock"; }

    std::any execute(const std::vector<std::any>& args) override {
        if (args.size() != 1) {
            throw std::invalid_argument( "BAD_ARGUMENTS: mock expects 1 argument");
        }
        return std::any_cast<int>(args[0]) * 2;
    }
};

TEST(DispatcherTest, RegisterAndDispatch) {
    iris::Dispatcher dispatcher;
    dispatcher.register_function(std::make_unique<MockFunc>());

    std::vector<std::any> args = {42};
    auto result = dispatcher.dispatch("mock", args);
    EXPECT_EQ(std::any_cast<int>(result), 84);
}

TEST(DispatcherTest, UnknownCommandThrows) {
    iris::Dispatcher dispatcher;
    dispatcher.register_function(std::make_unique<MockFunc>());

    std::vector<std::any> args;
    try {
        dispatcher.dispatch("unknown", args);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        EXPECT_TRUE(msg.find("UNKNOWN_COMMAND") != std::string::npos);
    }
}

TEST(DispatcherTest, BadArgumentsThrows) {
    iris::Dispatcher dispatcher;
    dispatcher.register_function(std::make_unique<MockFunc>());

    // Wrong number of arguments
    std::vector<std::any> args = {1, 2};
    try {
        dispatcher.dispatch("mock", args);
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument& e) {
        std::string msg = e.what();
        EXPECT_TRUE(msg.find("BAD_ARGUMENTS") != std::string::npos);
    }
}

TEST(DispatcherTest, EmptyCommandThrows) {
    iris::Dispatcher dispatcher;
    dispatcher.register_function(std::make_unique<MockFunc>());

    try {
        dispatcher.dispatch("", std::vector<std::any>{});
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        EXPECT_TRUE(msg.find("UNKNOWN_COMMAND") != std::string::npos);
    }
}
