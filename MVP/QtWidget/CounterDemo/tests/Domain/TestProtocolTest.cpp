#include "../../src/Domain/FormulaTemplate.hpp"
#include "../../src/Domain/TestProtocol.hpp"

#include <gtest/gtest.h>

namespace {

TEST(TestProtocolTest, ParsesKnownTestModeKeys) {
    EXPECT_EQ(domain::testModeFromKey("manual"), domain::TestMode::Manual);
    EXPECT_EQ(domain::testModeFromKey("hybrid"), domain::TestMode::Hybrid);
    EXPECT_EQ(domain::testModeFromKey("automatic"), domain::TestMode::Automatic);
}

TEST(TestProtocolTest, FallsBackToManualModeForUnknownKey) {
    EXPECT_EQ(domain::testModeFromKey("unexpected"), domain::TestMode::Manual);
}

TEST(TestProtocolTest, ParsesKnownProgramKeys) {
    EXPECT_EQ(domain::testProgramFromKey("custom"), domain::TestProgram::Custom);
    EXPECT_EQ(domain::testProgramFromKey("test1"), domain::TestProgram::StabilityInIdealConditions);
    EXPECT_EQ(domain::testProgramFromKey("test2"), domain::TestProgram::MaximumWindLoad);
    EXPECT_EQ(domain::testProgramFromKey("test3"), domain::TestProgram::WindLoadTemporalPerspective);
    EXPECT_EQ(domain::testProgramFromKey("attenuated_oscillation"), domain::TestProgram::AttenuatedOscillation);
}

TEST(TestProtocolTest, FallsBackToIdealConditionsProgramForUnknownKey) {
    EXPECT_EQ(domain::testProgramFromKey("unexpected"), domain::TestProgram::StabilityInIdealConditions);
}

TEST(TestProtocolTest, ExposesStableKeysForUiAndConfigBoundaries) {
    EXPECT_EQ(domain::testModeKey(domain::TestMode::Hybrid), "hybrid");
    EXPECT_EQ(domain::testProgramKey(domain::TestProgram::MaximumWindLoad), "test2");
}

TEST(TestProtocolTest, ExposesFormulaTemplatesForControlProfileCalculation) {
    EXPECT_EQ(domain::formulaTemplateByKey("calm").expression, "0");
    EXPECT_EQ(domain::formulaTemplateByKey("max_parameters").expression, "60 * sin(0.0053 * x)");
    EXPECT_EQ(domain::formulaTemplateByKey("temporal_perspective").expression, "sin(x) * (6.9 * sin(10 * x))");
    EXPECT_EQ(domain::formulaTemplateByKey("attenuated_oscillation").expression, "15 * abs(x * sin(x) / (1 + x**2))");
    EXPECT_EQ(domain::formulaTemplateKeyByExpression("0"), "calm");
}

TEST(TestProtocolTest, MapsTestProgramsToSingleScenarioFormula) {
    EXPECT_TRUE(domain::testProgramUsesCustomFormula(domain::TestProgram::Custom));
    EXPECT_FALSE(domain::testProgramUsesCustomFormula(domain::TestProgram::MaximumWindLoad));
    EXPECT_EQ(domain::formulaTemplateForTestProgram(domain::TestProgram::Custom).expression, "");
    EXPECT_EQ(domain::formulaTemplateForTestProgram(domain::TestProgram::StabilityInIdealConditions).expression, "0");
    EXPECT_EQ(domain::formulaTemplateForTestProgram(domain::TestProgram::MaximumWindLoad).expression,
              "60 * sin(0.0053 * x)");
    EXPECT_EQ(domain::formulaTemplateForTestProgram(domain::TestProgram::WindLoadTemporalPerspective).expression,
              "sin(x) * (6.9 * sin(10 * x))");
    EXPECT_EQ(domain::formulaTemplateForTestProgram(domain::TestProgram::AttenuatedOscillation).expression,
              "15 * abs(x * sin(x) / (1 + x**2))");
}

} // namespace
