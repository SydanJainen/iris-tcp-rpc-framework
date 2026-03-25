#include <gtest/gtest.h>

#include <any>
#include <stdexcept>
#include <string>
#include <vector>

#include "domain/to_base_func.h"

TEST(ToBaseFuncTest, Name) {
    iris::ToBaseFunc func;
    EXPECT_EQ(func.name(), "to_base");
}

TEST(ToBaseFuncTest, Hex255) {
    iris::ToBaseFunc func;
    auto result = func.execute({std::any(255), std::any(16)});
    EXPECT_EQ(std::any_cast<std::string>(result), "FF");
}

TEST(ToBaseFuncTest, Binary42) {
    iris::ToBaseFunc func;
    auto result = func.execute({std::any(42), std::any(2)});
    EXPECT_EQ(std::any_cast<std::string>(result), "101010");
}

TEST(ToBaseFuncTest, Decimal10) {
    iris::ToBaseFunc func;
    auto result = func.execute({std::any(10), std::any(10)});
    EXPECT_EQ(std::any_cast<std::string>(result), "10");
}

TEST(ToBaseFuncTest, Base36_35) {
    iris::ToBaseFunc func;
    auto result = func.execute({std::any(35), std::any(36)});
    EXPECT_EQ(std::any_cast<std::string>(result), "Z");
}

TEST(ToBaseFuncTest, Zero) {
    iris::ToBaseFunc func;
    auto result = func.execute({std::any(0), std::any(2)});
    EXPECT_EQ(std::any_cast<std::string>(result), "0");
}

TEST(ToBaseFuncTest, NegativeNumber) {
    iris::ToBaseFunc func;
    auto result = func.execute({std::any(-42), std::any(2)});
    EXPECT_EQ(std::any_cast<std::string>(result), "-101010");
}

TEST(ToBaseFuncTest, BaseTooLowThrows) {
    iris::ToBaseFunc func;
    try {
        func.execute({std::any(10), std::any(1)});
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument& e) {
        EXPECT_NE(std::string(e.what()).find("between 2 and 36"), std::string::npos);
    }
}

TEST(ToBaseFuncTest, BaseTooHighThrows) {
    iris::ToBaseFunc func;
    try {
        func.execute({std::any(10), std::any(37)});
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument& e) {
        EXPECT_NE(std::string(e.what()).find("between 2 and 36"), std::string::npos);
    }
}

TEST(ToBaseFuncTest, WrongArgCountThrows) {
    iris::ToBaseFunc func;
    EXPECT_THROW(func.execute({std::any(10)}), std::invalid_argument);
}

TEST(ToBaseFuncTest, WrongArgTypeThrows) {
    iris::ToBaseFunc func;
    EXPECT_THROW(func.execute({std::any(std::string("x")), std::any(2)}), std::invalid_argument);
}
