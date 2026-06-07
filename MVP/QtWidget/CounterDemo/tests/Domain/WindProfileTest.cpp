#include "../../src/Domain/WindProfile.hpp"

#include <gtest/gtest.h>

namespace {

TEST(WindProfileTest, ClampsBeaufortToOperationalRange) {
    domain::WindProfile low{};
    low.beaufort = -1.0;

    domain::WindProfile high{};
    high.beaufort = 12.0;

    EXPECT_DOUBLE_EQ(domain::sanitize(low).beaufort, domain::minOperationalBeaufort);
    EXPECT_DOUBLE_EQ(domain::sanitize(high).beaufort, domain::maxOperationalBeaufort);
}

TEST(WindProfileTest, NormalizesDirectionToCompassCircle) {
    domain::WindProfile negative{};
    negative.direction = -22.5;

    domain::WindProfile overflow{};
    overflow.direction = 382.5;

    EXPECT_DOUBLE_EQ(domain::sanitize(negative).direction, 337.5);
    EXPECT_DOUBLE_EQ(domain::sanitize(overflow).direction, 22.5);
}

TEST(WindProfileTest, ClampsAngleOfAttackToStandRange) {
    domain::WindProfile low{};
    low.angleOfAttack = -120.0;

    domain::WindProfile high{};
    high.angleOfAttack = 120.0;

    EXPECT_DOUBLE_EQ(domain::sanitize(low).angleOfAttack, -90.0);
    EXPECT_DOUBLE_EQ(domain::sanitize(high).angleOfAttack, 90.0);
}

} // namespace
