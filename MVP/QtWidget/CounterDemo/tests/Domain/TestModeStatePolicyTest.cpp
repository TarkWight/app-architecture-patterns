#include "../../src/Domain/TestModeStatePolicy.hpp"

#include <gtest/gtest.h>

namespace {

TEST(TestModeStatePolicyTest, MapsManualStandMode_WhenSelected_ReturnsManualFreeRunCountUpState) {
    // Arrange / Act
    const auto state = domain::TestModeStatePolicy::fromStandControlMode(domain::StandControlMode::Manual);

    // Assert
    EXPECT_EQ(state.testMode, domain::TestMode::Manual);
    EXPECT_EQ(state.standControlMode, domain::StandControlMode::Manual);
    EXPECT_EQ(state.timeSource, domain::TestTimeSource::FreeRun);
    EXPECT_EQ(state.timeDirection, domain::TestTimeDirection::CountUp);
}

TEST(TestModeStatePolicyTest, MapsHybridTestMode_WhenSelected_ReturnsHybridAutoCalculatedCountDownState) {
    // Arrange / Act
    const auto state = domain::TestModeStatePolicy::fromTestMode(domain::TestMode::Hybrid);

    // Assert
    EXPECT_EQ(state.testMode, domain::TestMode::Hybrid);
    EXPECT_EQ(state.standControlMode, domain::StandControlMode::Hybrid);
    EXPECT_EQ(state.timeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(state.timeDirection, domain::TestTimeDirection::CountDown);
}

TEST(TestModeStatePolicyTest, MapsAutomaticTestMode_WhenSelected_ReturnsPresetScenarioAutoCalculatedCountDownState) {
    // Arrange / Act
    const auto state = domain::TestModeStatePolicy::fromTestMode(domain::TestMode::Automatic);

    // Assert
    EXPECT_EQ(state.testMode, domain::TestMode::Automatic);
    EXPECT_EQ(state.standControlMode, domain::StandControlMode::PresetScenario);
    EXPECT_EQ(state.timeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(state.timeDirection, domain::TestTimeDirection::CountDown);
}

TEST(TestModeStatePolicyTest, MapsTimeSource_WhenAutoOrOperatorSelected_ReturnsCountDown) {
    // Arrange / Act / Assert
    EXPECT_EQ(domain::TestModeStatePolicy::directionForTimeSource(domain::TestTimeSource::AutoCalculated),
              domain::TestTimeDirection::CountDown);
    EXPECT_EQ(domain::TestModeStatePolicy::directionForTimeSource(domain::TestTimeSource::OperatorDefined),
              domain::TestTimeDirection::CountDown);
}

TEST(TestModeStatePolicyTest, MapsTimeSource_WhenFreeRunSelected_ReturnsCountUp) {
    // Arrange / Act / Assert
    EXPECT_EQ(domain::TestModeStatePolicy::directionForTimeSource(domain::TestTimeSource::FreeRun),
              domain::TestTimeDirection::CountUp);
}

} // namespace
