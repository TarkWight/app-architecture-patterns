#include "../../src/Domain/WindProfile.hpp"

#include <gtest/gtest.h>

namespace {

TEST(WindProfileTest, ClampsBeaufortToOperationalRange) {
    EXPECT_DOUBLE_EQ(domain::Beaufort::from(-1.0).value(), domain::minOperationalBeaufort);
    EXPECT_DOUBLE_EQ(domain::Beaufort::from(12.0).value(), domain::maxOperationalBeaufort);
}

TEST(WindProfileTest, NormalizesDirectionToCompassCircle) {
    EXPECT_DOUBLE_EQ(domain::WindDirection::from(-22.5).degrees(), 337.5);
    EXPECT_DOUBLE_EQ(domain::WindDirection::from(382.5).degrees(), 22.5);
}

TEST(WindProfileTest, ClampsAngleOfAttackToStandRange) {
    EXPECT_DOUBLE_EQ(domain::AngleOfAttack::from(-120.0).degrees(), domain::minAngleOfAttack);
    EXPECT_DOUBLE_EQ(domain::AngleOfAttack::from(120.0).degrees(), domain::maxAngleOfAttack);
}

TEST(WindProfileTest, FactoryCreatesProfileWithNormalizedValues) {
    const auto profile = domain::makeWindProfile(9.0, -45.0, 120.0);

    EXPECT_DOUBLE_EQ(profile.beaufort.value(), domain::maxOperationalBeaufort);
    EXPECT_DOUBLE_EQ(profile.direction.degrees(), 315.0);
    EXPECT_DOUBLE_EQ(profile.angleOfAttack.degrees(), domain::maxAngleOfAttack);
}

} // namespace
