#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "../src/container.h"

using iris::Container;

class IGreeter {
public:
    virtual ~IGreeter() = default;
    virtual std::string greet(const std::string& name) const = 0;
};


class HelloGreeter : public IGreeter {
public:
    std::string greet(const std::string& name) const override {
        return "Hello, " + name + "!";
    }
};


class CasualGreeter : public IGreeter {
public:
    std::string greet(const std::string& name) const override {
        return "Hey " + name;
    }
};


struct Config {
    std::string host;
    int port;
};

TEST(ContainerSingleton, ResolvesRegisteredSingleton) {
    Container c;
    c.register_singleton<Config>("config", []() {
        return Config{"localhost", 5555};
    });

    auto& cfg = c.resolve<Config>("config");
    EXPECT_EQ(cfg.host, "localhost");
    EXPECT_EQ(cfg.port, 5555);
}

TEST(ContainerSingleton, ReturnsSameInstanceOnMultipleResolves) {
    Container c;
    int call_count = 0;
    c.register_singleton<Config>("config", [&call_count]() {
        ++call_count;
        return Config{"127.0.0.1", 8080};
    });

    auto& first = c.resolve<Config>("config");
    auto& second = c.resolve<Config>("config");

    EXPECT_EQ(&first, &second);
    EXPECT_EQ(call_count, 1);
}

TEST(ContainerSingleton, WorksWithSharedPtr) {
    Container c;
    c.register_singleton<std::shared_ptr<IGreeter>>("greeter", []() {
        return std::shared_ptr<IGreeter>(std::make_shared<HelloGreeter>());
    });

    auto& greeter = c.resolve<std::shared_ptr<IGreeter>>("greeter");
    EXPECT_EQ(greeter->greet("World"), "Hello, World!");
}

// ---------------------------------------------------------------------------
// Factory tests
// ---------------------------------------------------------------------------

TEST(ContainerFactory, CreatesNewInstanceEachTime) {
    Container c;
    int call_count = 0;
    c.register_factory<Config>("config", [&call_count]() {
        ++call_count;
        return Config{"host_" + std::to_string(call_count), call_count};
    });

    auto& first = c.resolve<Config>("config");
    EXPECT_EQ(first.host, "host_1");

    auto& second = c.resolve<Config>("config");
    EXPECT_EQ(second.host, "host_2");

    EXPECT_EQ(call_count, 2);
}

// ---------------------------------------------------------------------------
// Error handling
// ---------------------------------------------------------------------------

TEST(ContainerError, ThrowsOnUnknownName) {
    Container c;
    EXPECT_THROW(c.resolve<Config>("nonexistent"), std::runtime_error);
}

TEST(ContainerError, ThrowsOnTypeMismatch) {
    Container c;
    c.register_singleton<Config>("config", []() {
        return Config{"localhost", 5555};
    });

    EXPECT_THROW(c.resolve<std::string>("config"), std::bad_any_cast);
}

// ---------------------------------------------------------------------------
// has() query
// ---------------------------------------------------------------------------

TEST(ContainerHas, ReturnsTrueForRegistered) {
    Container c;
    c.register_singleton<int>("answer", []() { return 42; });
    EXPECT_TRUE(c.has("answer"));
}

TEST(ContainerHas, ReturnsFalseForUnregistered) {
    Container c;
    EXPECT_FALSE(c.has("missing"));
}

// ---------------------------------------------------------------------------
// Mixed singleton and factory
// ---------------------------------------------------------------------------

TEST(ContainerMixed, SingletonAndFactoryCoexist) {
    Container c;

    c.register_singleton<Config>("db_config", []() {
        return Config{"db.local", 5432};
    });

    int seq = 0;
    c.register_factory<Config>("request_config", [&seq]() {
        ++seq;
        return Config{"req_" + std::to_string(seq), seq};
    });

    auto& db1 = c.resolve<Config>("db_config");
    auto& db2 = c.resolve<Config>("db_config");
    EXPECT_EQ(&db1, &db2);

    auto& req1 = c.resolve<Config>("request_config");
    EXPECT_EQ(req1.host, "req_1");

    auto& req2 = c.resolve<Config>("request_config");
    EXPECT_EQ(req2.host, "req_2");
}