#include "../../src/Domain/AngleOfAttackOscillationPolicy.hpp"

#include <gtest/gtest.h>

namespace {

TEST(AngleOfAttackPolicyTest, WhenBaseIs90AndAngleIs15_ReturnsSector82_5To97_5) {
    const auto sector = domain::AngleOfAttackOscillationPolicy::sector(domain::WindDirection::from(90.0),
                                                                       domain::AngleOfAttack::from(15.0));

    EXPECT_DOUBLE_EQ(sector.lower.degrees(), 82.5);
    EXPECT_DOUBLE_EQ(sector.upper.degrees(), 97.5);
}

TEST(AngleOfAttackPolicyTest, NormalizesSectorBoundsToZeroThreeSixtyRange) {
    const auto sector = domain::AngleOfAttackOscillationPolicy::sector(domain::WindDirection::from(355.0),
                                                                       domain::AngleOfAttack::from(20.0));

    EXPECT_DOUBLE_EQ(sector.lower.degrees(), 345.0);
    EXPECT_DOUBLE_EQ(sector.upper.degrees(), 5.0);
}

TEST(AngleOfAttackPolicyTest, WhenTargetBoundaryReached_ReturnsOppositeBoundary) {
    const auto first = domain::AngleOfAttackOscillationPolicy::nextTarget(
        domain::AngleOfAttackOscillationState{}, domain::WindDirection::from(90.0), domain::AngleOfAttack::from(15.0));

    const auto second = domain::AngleOfAttackOscillationPolicy::nextTarget(
        first.state, domain::WindDirection::from(90.0), domain::AngleOfAttack::from(15.0), first.target);

    EXPECT_DOUBLE_EQ(first.target.degrees(), 97.5);
    EXPECT_DOUBLE_EQ(second.target.degrees(), 82.5);
}

TEST(AngleOfAttackPolicyTest, WhenTargetBoundaryIsNotReached_KeepsCurrentTarget) {
    const auto first = domain::AngleOfAttackOscillationPolicy::nextTarget(
        domain::AngleOfAttackOscillationState{}, domain::WindDirection::from(90.0), domain::AngleOfAttack::from(15.0));

    const auto second = domain::AngleOfAttackOscillationPolicy::nextTarget(
        first.state, domain::WindDirection::from(90.0), domain::AngleOfAttack::from(15.0),
        domain::WindDirection::from(90.0));

    EXPECT_DOUBLE_EQ(first.target.degrees(), 97.5);
    EXPECT_DOUBLE_EQ(second.target.degrees(), 97.5);
}

} // namespace
