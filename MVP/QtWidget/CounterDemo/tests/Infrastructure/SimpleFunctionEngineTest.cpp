#include "../../src/Infrastructure/SimpleFunctionEngine.hpp"

#include <gtest/gtest.h>

#include <cmath>

namespace {

TEST(SimpleFunctionEngineTest, EvaluatesArithmeticWithPrecedence) {
    const infrastructure::SimpleFunctionEngine engine{};

    EXPECT_DOUBLE_EQ(engine.eval("1 + 2 * 3", 0.0), 7.0);
    EXPECT_DOUBLE_EQ(engine.eval("(1 + 2) * 3", 0.0), 9.0);
}

TEST(SimpleFunctionEngineTest, EvaluatesVariableAliases) {
    const infrastructure::SimpleFunctionEngine engine{};

    EXPECT_DOUBLE_EQ(engine.eval("x * 2", 4.0), 8.0);
    EXPECT_DOUBLE_EQ(engine.eval("t + 3", 4.0), 7.0);
}

TEST(SimpleFunctionEngineTest, EvaluatesFunctionsAndPower) {
    const infrastructure::SimpleFunctionEngine engine{};

    EXPECT_DOUBLE_EQ(engine.eval("x^2", 3.0), 9.0);
    EXPECT_DOUBLE_EQ(engine.eval("x**2", 3.0), 9.0);
    EXPECT_NEAR(engine.eval("sin(x)", 1.0), std::sin(1.0), 0.000001);
    EXPECT_NEAR(engine.eval("cos(x)", 1.0), std::cos(1.0), 0.000001);
    EXPECT_DOUBLE_EQ(engine.eval("abs(x)", -3.0), 3.0);
}

TEST(SimpleFunctionEngineTest, ReturnsZeroForInvalidExpressions) {
    const infrastructure::SimpleFunctionEngine engine{};

    EXPECT_DOUBLE_EQ(engine.eval("sin x", 1.0), 0.0);
    EXPECT_DOUBLE_EQ(engine.eval("1 / 0", 1.0), 0.0);
    EXPECT_DOUBLE_EQ(engine.eval("1..2", 1.0), 0.0);
}

} // namespace
