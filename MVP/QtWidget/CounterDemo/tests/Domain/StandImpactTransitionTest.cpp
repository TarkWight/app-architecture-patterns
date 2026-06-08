#include "../../src/Domain/StandImpactTransition.hpp"

#include <gtest/gtest.h>

namespace {

TEST(StandImpactTransitionTest, AdvancesImpactByConfiguredSafeSteps) {
    const auto current = domain::makeWindImpact(1.0, 10.0, 5.0);
    const auto target = domain::makeWindImpact(2.0, 20.0, 15.0);

    const auto result = domain::StandImpactTransition{}.advance(current, target);

    EXPECT_DOUBLE_EQ(result.impact.beaufort.value(), 1.1);
    EXPECT_DOUBLE_EQ(result.impact.direction.degrees(), 12.5);
    EXPECT_DOUBLE_EQ(result.impact.angleOfAttack.degrees(), 6.0);
    EXPECT_FALSE(result.targetReached);
}

TEST(StandImpactTransitionTest, SnapsToTargetWhenRemainingDeltaFitsIntoOneStep) {
    const auto current = domain::makeWindImpact(1.95, 18.0, 4.5);
    const auto target = domain::makeWindImpact(2.0, 20.0, 5.0);

    const auto result = domain::StandImpactTransition{}.advance(current, target);

    EXPECT_DOUBLE_EQ(result.impact.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(result.impact.direction.degrees(), 20.0);
    EXPECT_DOUBLE_EQ(result.impact.angleOfAttack.degrees(), 5.0);
    EXPECT_TRUE(result.targetReached);
}

TEST(StandImpactTransitionTest, CarriesTargetFormulaToNextImpact) {
    auto current = domain::makeWindImpact(0.0, 0.0, 0.0);
    current.formula.value = "old";

    auto target = domain::makeWindImpact(0.0, 0.0, 0.0);
    target.formula.value = "new";

    const auto result = domain::StandImpactTransition{}.advance(current, target);

    EXPECT_EQ(result.impact.formula.value, "new");
    EXPECT_TRUE(result.targetReached);
}

} // namespace
