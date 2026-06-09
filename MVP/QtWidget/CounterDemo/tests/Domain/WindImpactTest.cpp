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

TEST(WindImpactTest, ClampsAngleOfAttackToStandRange) {
    EXPECT_DOUBLE_EQ(domain::AngleOfAttack::from(-120.0).degrees(), domain::minAngleOfAttack);
    EXPECT_DOUBLE_EQ(domain::AngleOfAttack::from(400.0).degrees(), domain::maxAngleOfAttack);
}

TEST(WindImpactTest, FactoryCreatesImpactWithNormalizedValues) {
    const auto impact = domain::makeWindImpact(9.0, -45.0, 400.0);

    EXPECT_DOUBLE_EQ(impact.beaufort.value(), domain::maxOperationalBeaufort);
    EXPECT_DOUBLE_EQ(impact.direction.degrees(), 315.0);
    EXPECT_DOUBLE_EQ(impact.angleOfAttack.degrees(), domain::maxAngleOfAttack);
}

} // namespace
