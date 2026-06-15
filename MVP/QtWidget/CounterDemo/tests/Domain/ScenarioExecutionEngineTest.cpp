#include "../../src/Domain/ScenarioExecutionEngine.hpp"

#include <gtest/gtest.h>

namespace {

domain::WindControlProfile profileWithSamples() {
    return domain::WindControlProfile{
        .duration = domain::DurationMinutes::required(1),
        .sampleIntervalSeconds = 1.0,
        .samples =
            {
                domain::WindControlSample{.time = domain::WindControlSampleTime::fromSeconds(0.0),
                                          .beaufort = domain::Beaufort::from(1.0)},
                domain::WindControlSample{.time = domain::WindControlSampleTime::fromSeconds(1.0),
                                          .beaufort = domain::Beaufort::from(2.0)},
            },
    };
}

TEST(ScenarioExecutionEngineTest, AdvancesScenario_WhenProfileHasSample_ReturnsImpactAndTraceSample) {
    // Arrange
    const auto profile = profileWithSamples();
    const auto baseImpact = domain::makeWindImpact(0.0, 90.0, 12.0);

    // Act
    const auto step = domain::ScenarioExecutionEngine::advance(profile, domain::ElapsedSeconds::from(1), baseImpact);

    // Assert
    ASSERT_TRUE(step.has_value());
    EXPECT_DOUBLE_EQ(step->impact.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(step->impact.direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(step->impact.angleOfAttack.degrees(), 12.0);
    EXPECT_DOUBLE_EQ(step->traceSample.time.seconds(), 1.0);
    EXPECT_DOUBLE_EQ(step->traceSample.targetValue.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(step->traceSample.safeCommandValue.beaufort.value(), 2.0);
}

TEST(ScenarioExecutionEngineTest, AdvancesScenario_WhenElapsedExceedsProfile_ReturnsLastImpactSample) {
    // Arrange
    const auto profile = profileWithSamples();
    const auto baseImpact = domain::makeWindImpact(0.0, 180.0, 5.0);

    // Act
    const auto step = domain::ScenarioExecutionEngine::advance(profile, domain::ElapsedSeconds::from(120), baseImpact);

    // Assert
    ASSERT_TRUE(step.has_value());
    EXPECT_DOUBLE_EQ(step->impact.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(step->impact.direction.degrees(), 180.0);
    EXPECT_DOUBLE_EQ(step->impact.angleOfAttack.degrees(), 5.0);
    EXPECT_DOUBLE_EQ(step->traceSample.time.seconds(), 120.0);
}

TEST(ScenarioExecutionEngineTest, AdvancesScenario_WhenProfileIsEmpty_ReturnsNoStep) {
    // Arrange
    const domain::WindControlProfile profile{};
    const auto baseImpact = domain::makeWindImpact(0.0, 0.0, 0.0);

    // Act
    const auto step = domain::ScenarioExecutionEngine::advance(profile, domain::ElapsedSeconds::from(0), baseImpact);

    // Assert
    EXPECT_FALSE(step.has_value());
}

} // namespace
