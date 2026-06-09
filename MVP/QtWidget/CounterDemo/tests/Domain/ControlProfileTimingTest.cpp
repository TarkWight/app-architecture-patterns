#include "../../src/Domain/ControlProfileTiming.hpp"

#include <gtest/gtest.h>

namespace {

TEST(ControlProfileTimingTest, ManualModeDoesNotUseFormulaProfile) {
    const auto timing = domain::determineControlProfileTiming(
        domain::TestMode::Manual, domain::TestTimeSource::OperatorDefined, domain::DurationMinutes::required(30),
        domain::DurationMinutes::required(12));

    EXPECT_FALSE(timing.formulaEnabled);
    EXPECT_EQ(timing.duration.value(), 0);
}

TEST(ControlProfileTimingTest, AutomaticModeUsesCalculatedDuration) {
    const auto timing = domain::determineControlProfileTiming(
        domain::TestMode::Automatic, domain::TestTimeSource::OperatorDefined, domain::DurationMinutes::required(30),
        domain::DurationMinutes::required(12));

    EXPECT_TRUE(timing.formulaEnabled);
    EXPECT_EQ(timing.duration.value(), 30);
}

TEST(ControlProfileTimingTest, HybridModeUsesOperatorDurationOnlyWhenOperatorSelectedIt) {
    const auto calculatedTiming = domain::determineControlProfileTiming(
        domain::TestMode::Hybrid, domain::TestTimeSource::AutoCalculated, domain::DurationMinutes::required(30),
        domain::DurationMinutes::required(12));
    const auto operatorTiming = domain::determineControlProfileTiming(
        domain::TestMode::Hybrid, domain::TestTimeSource::OperatorDefined, domain::DurationMinutes::required(30),
        domain::DurationMinutes::required(12));

    EXPECT_EQ(calculatedTiming.duration.value(), 30);
    EXPECT_EQ(operatorTiming.duration.value(), 12);
}

} // namespace
