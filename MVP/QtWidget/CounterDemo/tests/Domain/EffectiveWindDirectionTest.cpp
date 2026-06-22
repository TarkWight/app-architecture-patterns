#include "../../src/Domain/EffectiveWindDirection.hpp"

#include <gtest/gtest.h>

namespace {

TEST(EffectiveWindDirectionTest, AddsAngleOfAttackToWindDirection) {
    const auto effectiveDirection =
        domain::EffectiveWindDirection::from(domain::WindDirection::from(70.0), domain::AngleOfAttack::from(30.0));

    EXPECT_DOUBLE_EQ(effectiveDirection.degrees(), 100.0);
}

TEST(EffectiveWindDirectionTest, NormalizesDirectionAcrossZero) {
    const auto effectiveDirection =
        domain::EffectiveWindDirection::from(domain::WindDirection::from(270.0), domain::AngleOfAttack::from(90.0));

    EXPECT_DOUBLE_EQ(effectiveDirection.degrees(), 0.0);
}

TEST(EffectiveWindDirectionTest, SupportsNegativeAngleOfAttack) {
    const auto effectiveDirection =
        domain::EffectiveWindDirection::from(domain::WindDirection::from(90.0), domain::AngleOfAttack::from(-15.0));

    EXPECT_DOUBLE_EQ(effectiveDirection.degrees(), 75.0);
}

TEST(EffectiveWindDirectionTest, NormalizesPositiveOverflow) {
    const auto effectiveDirection =
        domain::EffectiveWindDirection::from(domain::WindDirection::from(350.0), domain::AngleOfAttack::from(20.0));

    EXPECT_DOUBLE_EQ(effectiveDirection.degrees(), 10.0);
}

TEST(EffectiveWindDirectionTest, AddsPositiveYawOscillationOffset) {
    const auto effectiveDirection = domain::EffectiveWindDirection::from(
        domain::WindDirection::from(70.0), domain::AngleOfAttack::from(30.0), domain::YawOscillationOffset::from(5.0));

    EXPECT_DOUBLE_EQ(effectiveDirection.degrees(), 105.0);
}

TEST(EffectiveWindDirectionTest, AddsNegativeYawOscillationOffset) {
    const auto effectiveDirection = domain::EffectiveWindDirection::from(
        domain::WindDirection::from(70.0), domain::AngleOfAttack::from(30.0), domain::YawOscillationOffset::from(-5.0));

    EXPECT_DOUBLE_EQ(effectiveDirection.degrees(), 95.0);
}

} // namespace
