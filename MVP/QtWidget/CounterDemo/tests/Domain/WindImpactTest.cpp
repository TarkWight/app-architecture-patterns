#include "../../src/Domain/WindImpact.hpp"

#include <gtest/gtest.h>

namespace {

TEST(WindImpactTest, ClampsBeaufortToOperationalRange) {
    EXPECT_DOUBLE_EQ(domain::Beaufort::from(-1.0).value(), domain::minOperationalBeaufort);
    EXPECT_DOUBLE_EQ(domain::Beaufort::from(12.0).value(), domain::maxOperationalBeaufort);
}

TEST(WindImpactTest, NormalizesDirectionToCompassCircle) {
    EXPECT_DOUBLE_EQ(domain::WindDirection::from(-22.5).degrees(), 337.5);
    EXPECT_DOUBLE_EQ(domain::WindDirection::from(382.5).degrees(), 22.5);
}

TEST(WindImpactTest, NormalizesDirectionNearZeroWithoutReturningThreeSixty) {
    EXPECT_DOUBLE_EQ(domain::WindDirection::from(-0.0000001).degrees(), 0.0);
    EXPECT_DOUBLE_EQ(domain::WindDirection::from(360.0).degrees(), 0.0);
    EXPECT_DOUBLE_EQ(domain::WindDirection::from(720.0).degrees(), 0.0);
    EXPECT_DOUBLE_EQ(domain::WindDirection::from(-5.0).degrees(), 355.0);
    EXPECT_DOUBLE_EQ(domain::WindDirection::from(359.999999999).degrees(), 0.0);
}

TEST(WindImpactTest, ClampsAngleOfAttackToSignedStandRange) {
    EXPECT_DOUBLE_EQ(domain::AngleOfAttack::from(-720.0).degrees(), domain::minAngleOfAttack);
    EXPECT_DOUBLE_EQ(domain::AngleOfAttack::from(400.0).degrees(), domain::maxAngleOfAttack);
    EXPECT_DOUBLE_EQ(domain::AngleOfAttack::from(-15.0).degrees(), -15.0);
}

TEST(WindImpactTest, FactoryCreatesImpactWithNormalizedValues) {
    const auto impact = domain::makeWindImpact(9.0, -45.0, -15.0);

    EXPECT_DOUBLE_EQ(impact.beaufort.value(), domain::maxOperationalBeaufort);
    EXPECT_DOUBLE_EQ(impact.direction.degrees(), 315.0);
    EXPECT_DOUBLE_EQ(impact.angleOfAttack.degrees(), -15.0);
}

TEST(WindImpactTest, WithBeaufortChangesOnlyBeaufort) {
    const auto original = domain::makeWindImpact(2.0, 90.0, 15.0);
    const auto updated = original.withBeaufort(domain::Beaufort::from(6.0));

    EXPECT_DOUBLE_EQ(updated.beaufort.value(), 6.0);
    EXPECT_DOUBLE_EQ(updated.direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(updated.angleOfAttack.degrees(), 15.0);
}

TEST(WindImpactTest, WithDirectionChangesOnlyDirection) {
    const auto original = domain::makeWindImpact(2.0, 90.0, 15.0);
    const auto updated = original.withDirection(domain::WindDirection::from(270.0));

    EXPECT_DOUBLE_EQ(updated.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(updated.direction.degrees(), 270.0);
    EXPECT_DOUBLE_EQ(updated.angleOfAttack.degrees(), 15.0);
}

TEST(WindImpactTest, WithAngleOfAttackChangesOnlyAngleOfAttack) {
    const auto original = domain::makeWindImpact(2.0, 90.0, 15.0);
    const auto updated = original.withAngleOfAttack(domain::AngleOfAttack::from(45.0));

    EXPECT_DOUBLE_EQ(updated.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(updated.direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(updated.angleOfAttack.degrees(), 45.0);
}

} // namespace
