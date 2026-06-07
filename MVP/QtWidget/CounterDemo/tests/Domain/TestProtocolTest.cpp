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
    EXPECT_EQ(domain::testProgramFromKey("test1"), domain::TestProgram::StabilityInIdealConditions);
    EXPECT_EQ(domain::testProgramFromKey("test2"), domain::TestProgram::MaximumWindLoad);
    EXPECT_EQ(domain::testProgramFromKey("test3"), domain::TestProgram::WindLoadTemporalPerspective);
}

TEST(TestProtocolTest, FallsBackToIdealConditionsProgramForUnknownKey) {
    EXPECT_EQ(domain::testProgramFromKey("unexpected"), domain::TestProgram::StabilityInIdealConditions);
}

TEST(TestProtocolTest, ExposesStableKeysForUiAndConfigBoundaries) {
    EXPECT_EQ(domain::testModeKey(domain::TestMode::Hybrid), "hybrid");
    EXPECT_EQ(domain::testProgramKey(domain::TestProgram::MaximumWindLoad), "test2");
}

} // namespace
