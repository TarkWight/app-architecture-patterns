#include "../../src/Domain/StandScenario.hpp"

#include <gtest/gtest.h>

namespace {

TEST(StandScenarioTest, ManualModeKeepsManualTarget) {
    const domain::StandScenario scenario{domain::StandControlMode::Manual};

    EXPECT_TRUE(scenario.allowsManualImpact());
    EXPECT_FALSE(scenario.locksManualControls());
    EXPECT_EQ(scenario.manualImpactPolicy(), domain::ManualImpactPolicy::KeepManualTarget);
}

TEST(StandScenarioTest, HybridModeAllowsTemporaryManualImpact) {
    const domain::StandScenario scenario{domain::StandControlMode::Hybrid};

    EXPECT_TRUE(scenario.allowsManualImpact());
    EXPECT_FALSE(scenario.locksManualControls());
    EXPECT_EQ(scenario.manualImpactPolicy(), domain::ManualImpactPolicy::ReturnToScenarioAfterManualImpact);
}

TEST(StandScenarioTest, PresetScenarioModeRejectsManualImpact) {
    const domain::StandScenario scenario{domain::StandControlMode::PresetScenario};

    EXPECT_FALSE(scenario.allowsManualImpact());
    EXPECT_TRUE(scenario.locksManualControls());
    EXPECT_EQ(scenario.manualImpactPolicy(), domain::ManualImpactPolicy::RejectManualImpact);
}

} // namespace
