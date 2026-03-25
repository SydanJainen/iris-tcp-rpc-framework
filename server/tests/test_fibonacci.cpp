#include <gtest/gtest.h>

#include <any>
#include <stdexcept>
#include <string>
#include <vector>

#include "domain/fibonacci_func.h"

TEST(FibonacciFuncTest, Name) {
    iris::FibonacciFunc func;
    EXPECT_EQ(func.name(), "fibonacci");
}

TEST(FibonacciFuncTest, Fib0) {
    iris::FibonacciFunc func;
    auto result = func.execute({std::any(0)});
    EXPECT_EQ(std::any_cast<long long>(result), 0LL);
}

TEST(FibonacciFuncTest, Fib1) {
    iris::FibonacciFunc func;
    auto result = func.execute({std::any(1)});
    EXPECT_EQ(std::any_cast<long long>(result), 1LL);
}

TEST(FibonacciFuncTest, Fib2) {
    iris::FibonacciFunc func;
    auto result = func.execute({std::any(2)});
    EXPECT_EQ(std::any_cast<long long>(result), 1LL);
}

TEST(FibonacciFuncTest, Fib10) {
    iris::FibonacciFunc func;
    auto result = func.execute({std::any(10)});
    EXPECT_EQ(std::any_cast<long long>(result), 55LL);
}

TEST(FibonacciFuncTest, Fib20) {
    iris::FibonacciFunc func;
    auto result = func.execute({std::any(20)});
    EXPECT_EQ(std::any_cast<long long>(result), 6765LL);
}

TEST(FibonacciFuncTest, NegativeThrows) {
    iris::FibonacciFunc func;
    try {
        func.execute({std::any(-1)});
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument& e) {
        EXPECT_NE(std::string(e.what()).find("non-negative"), std::string::npos);
    }
}

TEST(FibonacciFuncTest, WrongArgCountThrows) {
    iris::FibonacciFunc func;
    EXPECT_THROW(func.execute({}), std::invalid_argument);
    EXPECT_THROW(func.execute({std::any(1), std::any(2)}), std::invalid_argument);
}

TEST(FibonacciFuncTest, WrongArgTypeThrows) {
    iris::FibonacciFunc func;
    EXPECT_THROW(func.execute({std::any(std::string("hello"))}), std::invalid_argument);
}
