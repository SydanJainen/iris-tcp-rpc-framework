#include <gtest/gtest.h>

#include <any>
#include <stdexcept>
#include <string>
#include <vector>

#include "domain/add_func.h"
#include "domain/reverse_func.h"
#include "domain/multiply_func.h"

TEST(AddFuncTest, Name) {
    iris::AddFunc func;
    EXPECT_EQ(func.name(), "add");
}

TEST(AddFuncTest, AddsPositiveNumbers) {
    iris::AddFunc func;
    auto result = func.execute({std::any(3), std::any(4)});
    EXPECT_EQ(std::any_cast<int>(result), 7);
}

TEST(AddFuncTest, AddsNegativeNumbers) {
    iris::AddFunc func;
    auto result = func.execute({std::any(-5), std::any(-3)});
    EXPECT_EQ(std::any_cast<int>(result), -8);
}

TEST(AddFuncTest, AddsZeros) {
    iris::AddFunc func;
    auto result = func.execute({std::any(0), std::any(0)});
    EXPECT_EQ(std::any_cast<int>(result), 0);
}

TEST(AddFuncTest, AddsMixedSign) {
    iris::AddFunc func;
    auto result = func.execute({std::any(10), std::any(-3)});
    EXPECT_EQ(std::any_cast<int>(result), 7);
}

TEST(AddFuncTest, WrongArgCountThrows) {
    iris::AddFunc func;
    EXPECT_THROW(func.execute({std::any(1)}), std::invalid_argument);
    EXPECT_THROW(func.execute({std::any(1), std::any(2), std::any(3)}),std::invalid_argument);
}

TEST(AddFuncTest, WrongArgTypeThrows) {
    iris::AddFunc func;
    EXPECT_THROW(
        func.execute({std::any(std::string("hello")), std::any(1)}),std::invalid_argument);
}


TEST(ReverseFuncTest, Name) {
    iris::ReverseFunc func;
    EXPECT_EQ(func.name(), "reverse");
}

TEST(ReverseFuncTest, ReversesString) {
    iris::ReverseFunc func;
    auto result = func.execute({std::any(std::string("hello"))});
    EXPECT_EQ(std::any_cast<std::string>(result), "olleh");
}

TEST(ReverseFuncTest, EmptyString) {
    iris::ReverseFunc func;
    auto result = func.execute({std::any(std::string(""))});
    EXPECT_EQ(std::any_cast<std::string>(result), "");
}

TEST(ReverseFuncTest, SingleChar) {
    iris::ReverseFunc func;
    auto result = func.execute({std::any(std::string("x"))});
    EXPECT_EQ(std::any_cast<std::string>(result), "x");
}

TEST(ReverseFuncTest, Palindrome) {
    iris::ReverseFunc func;
    auto result = func.execute({std::any(std::string("racecar"))});
    EXPECT_EQ(std::any_cast<std::string>(result), "racecar");
}

TEST(ReverseFuncTest, WrongArgCountThrows) {
    iris::ReverseFunc func;
    EXPECT_THROW(func.execute({}), std::invalid_argument);
    EXPECT_THROW(func.execute({std::any(std::string("a")), std::any(std::string("b"))}),std::invalid_argument);
}

TEST(ReverseFuncTest, WrongArgTypeThrows) {
    iris::ReverseFunc func;
    EXPECT_THROW(func.execute({std::any(42)}), std::invalid_argument);
}

TEST(MultiplyFuncTest, Name) {
    iris::MultiplyFunc func;
    EXPECT_EQ(func.name(), "multiply");
}

TEST(MultiplyFuncTest, MultipliesPositiveDoubles) {
    iris::MultiplyFunc func;
    auto result = func.execute({std::any(3.0), std::any(4.5)});
    EXPECT_DOUBLE_EQ(std::any_cast<double>(result), 13.5);
}

TEST(MultiplyFuncTest, MultipliesByZero) {
    iris::MultiplyFunc func;
    auto result = func.execute({std::any(999.0), std::any(0.0)});
    EXPECT_DOUBLE_EQ(std::any_cast<double>(result), 0.0);
}

TEST(MultiplyFuncTest, MultipliesNegatives) {
    iris::MultiplyFunc func;
    auto result = func.execute({std::any(-2.0), std::any(-3.0)});
    EXPECT_DOUBLE_EQ(std::any_cast<double>(result), 6.0);
}

TEST(MultiplyFuncTest, MultipliesMixedSign) {
    iris::MultiplyFunc func;
    auto result = func.execute({std::any(5.0), std::any(-2.0)});
    EXPECT_DOUBLE_EQ(std::any_cast<double>(result), -10.0);
}

TEST(MultiplyFuncTest, WrongArgCountThrows) {
    iris::MultiplyFunc func;
    EXPECT_THROW(func.execute({std::any(1.0)}), std::invalid_argument);
}

TEST(MultiplyFuncTest, WrongArgTypeThrows) {
    iris::MultiplyFunc func;
    EXPECT_THROW(func.execute({std::any(1), std::any(2)}),std::invalid_argument);
}
