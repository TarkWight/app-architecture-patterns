#include "../../src/Domain/WindControlProfileWorstCase.hpp"

#include <gtest/gtest.h>

namespace {

domain::WindControlSample sample(double seconds, double beaufort) {
    return domain::WindControlSample{.time = domain::WindControlSampleTime::fromSeconds(seconds),
                                     .beaufort = domain::Beaufort::from(beaufort)};
}

} // namespace

TEST(WindControlProfileWorstCaseTest, EmptyProfileReturnsFallbackImpact) {
    const auto fallback = domain::makeWindImpact(2.0, 90.0, -15.0);

    const auto result = domain::WindControlProfileWorstCase::from(domain::WindControlProfile{}, fallback);

    EXPECT_FALSE(result.usedProfile);
    EXPECT_DOUBLE_EQ(result.impact.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(result.impact.direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(result.impact.angleOfAttack.degrees(), -15.0);
}

TEST(WindControlProfileWorstCaseTest, ReturnsMaximumProfileBeaufortAndKeepsBaseDirectionAndAngle) {
    domain::WindControlProfile profile{};
    profile.samples = {sample(0.0, 2.0), sample(1.0, 6.0), sample(2.0, 4.0)};
    const auto fallback = domain::makeWindImpact(1.0, 270.0, -30.0);

    const auto result = domain::WindControlProfileWorstCase::from(profile, fallback);

    EXPECT_TRUE(result.usedProfile);
    EXPECT_DOUBLE_EQ(result.impact.beaufort.value(), 6.0);
    EXPECT_DOUBLE_EQ(result.impact.direction.degrees(), 270.0);
    EXPECT_DOUBLE_EQ(result.impact.angleOfAttack.degrees(), -30.0);
}

TEST(WindControlProfileWorstCaseTest, KeepsFallbackWhenProfileIsLowerThanCurrentImpact) {
    domain::WindControlProfile profile{};
    profile.samples = {sample(0.0, 1.0), sample(1.0, 2.0)};
    const auto fallback = domain::makeWindImpact(4.0, 180.0, 10.0);

    const auto result = domain::WindControlProfileWorstCase::from(profile, fallback);

    EXPECT_FALSE(result.usedProfile);
    EXPECT_DOUBLE_EQ(result.impact.beaufort.value(), 4.0);
    EXPECT_DOUBLE_EQ(result.impact.direction.degrees(), 180.0);
    EXPECT_DOUBLE_EQ(result.impact.angleOfAttack.degrees(), 10.0);
}
