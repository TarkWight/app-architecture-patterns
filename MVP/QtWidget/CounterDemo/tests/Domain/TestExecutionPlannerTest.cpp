#include "../../src/Domain/TestExecutionPlanner.hpp"

#include <gtest/gtest.h>

namespace {

domain::TestProtocol protocolWithMode(domain::TestMode mode) {
    domain::TestProtocol protocol{};
    protocol.testMode = mode;
    return protocol;
}

TEST(TestExecutionPlannerTest, PlansManualMode_WhenOperatorDurationProvided_ReturnsCountUpWithoutDuration) {
    // Arrange
    const auto protocol = protocolWithMode(domain::TestMode::Manual);

    // Act
    const auto plan = domain::TestExecutionPlanner::plan(protocol, domain::TestTimeSource::OperatorDefined,
                                                         domain::DurationMinutes::required(12),
                                                         domain::DurationMinutes::required(20));

    // Assert
    EXPECT_EQ(plan.direction, domain::TestTimeDirection::CountUp);
    EXPECT_EQ(plan.activeDuration.value(), 0);
    EXPECT_EQ(plan.initialRemaining().value(), 0);
    EXPECT_EQ(plan.remainingAt(domain::ElapsedSeconds::from(120)).value(), 0);
    EXPECT_FALSE(plan.isCompletedAt(domain::ElapsedSeconds::from(120)));
}

TEST(TestExecutionPlannerTest, PlansAutomaticMode_WhenAutoCalculatedTimeSelected_ReturnsCountdownForEstimatedDuration) {
    // Arrange
    const auto protocol = protocolWithMode(domain::TestMode::Automatic);

    // Act
    const auto plan = domain::TestExecutionPlanner::plan(protocol, domain::TestTimeSource::AutoCalculated,
                                                         domain::DurationMinutes::required(48),
                                                         domain::DurationMinutes::required(20));

    // Assert
    EXPECT_EQ(plan.direction, domain::TestTimeDirection::CountDown);
    EXPECT_EQ(plan.activeDuration.value(), 48);
    EXPECT_EQ(plan.initialRemaining().value(), 2880);
    EXPECT_EQ(plan.remainingAt(domain::ElapsedSeconds::from(60)).value(), 2820);
    EXPECT_FALSE(plan.isCompletedAt(domain::ElapsedSeconds::from(2879)));
    EXPECT_TRUE(plan.isCompletedAt(domain::ElapsedSeconds::from(2880)));
}

TEST(TestExecutionPlannerTest, PlansHybridMode_WhenOperatorTimeSelected_ReturnsCountdownForOperatorDuration) {
    // Arrange
    const auto protocol = protocolWithMode(domain::TestMode::Hybrid);

    // Act
    const auto plan = domain::TestExecutionPlanner::plan(protocol, domain::TestTimeSource::OperatorDefined,
                                                         domain::DurationMinutes::required(48),
                                                         domain::DurationMinutes::required(15));

    // Assert
    EXPECT_EQ(plan.direction, domain::TestTimeDirection::CountDown);
    EXPECT_EQ(plan.activeDuration.value(), 15);
    EXPECT_EQ(plan.initialRemaining().value(), 900);
    EXPECT_EQ(plan.remainingAt(domain::ElapsedSeconds::from(901)).value(), 0);
    EXPECT_TRUE(plan.isCompletedAt(domain::ElapsedSeconds::from(901)));
}

TEST(TestExecutionPlannerTest, PlansScenarioMode_WhenFreeRunSelected_ReturnsCountUpWithoutCompletion) {
    // Arrange
    const auto protocol = protocolWithMode(domain::TestMode::Hybrid);

    // Act
    const auto plan = domain::TestExecutionPlanner::plan(protocol, domain::TestTimeSource::FreeRun,
                                                         domain::DurationMinutes::required(48),
                                                         domain::DurationMinutes::required(15));

    // Assert
    EXPECT_EQ(plan.direction, domain::TestTimeDirection::CountUp);
    EXPECT_EQ(plan.activeDuration.value(), 0);
    EXPECT_EQ(plan.initialRemaining().value(), 0);
    EXPECT_FALSE(plan.isCompletedAt(domain::ElapsedSeconds::from(3600)));
}

TEST(TestExecutionPlannerTest, ResetsCountdownAfterStop_WhenDurationIsActive_ReturnsFullRemainingTime) {
    // Arrange / Act
    const auto stopPlan = domain::TestExecutionPlanner::resetAfterStop(domain::DurationMinutes::required(12),
                                                                       domain::TestTimeDirection::CountDown);

    // Assert
    EXPECT_EQ(stopPlan.elapsed.value(), 0);
    EXPECT_EQ(stopPlan.remaining.value(), 720);
}

TEST(TestExecutionPlannerTest, ResetsCountUpAfterStop_WhenManualTimerIsActive_ReturnsZeroRemainingTime) {
    // Arrange / Act
    const auto stopPlan = domain::TestExecutionPlanner::resetAfterStop(domain::DurationMinutes::required(12),
                                                                       domain::TestTimeDirection::CountUp);

    // Assert
    EXPECT_EQ(stopPlan.elapsed.value(), 0);
    EXPECT_EQ(stopPlan.remaining.value(), 0);
}

} // namespace
