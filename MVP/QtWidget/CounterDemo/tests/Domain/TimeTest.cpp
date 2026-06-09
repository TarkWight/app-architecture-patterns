#include "../../src/Domain/Time.hpp"

#include <gtest/gtest.h>

namespace {

TEST(TimeTest, RequiredDurationMinutesAreClampedToOperationalRange) {
    EXPECT_EQ(domain::DurationMinutes::required(-5).value(), domain::minDurationMinutes);
    EXPECT_EQ(domain::DurationMinutes::required(24 * 60 + 10).value(), domain::maxDurationMinutes);
}

TEST(TimeTest, OptionalDurationMinutesMayBeZeroButNotNegative) {
    EXPECT_EQ(domain::DurationMinutes::optional(-5).value(), 0);
    EXPECT_EQ(domain::DurationMinutes::optional(0).value(), 0);
    EXPECT_EQ(domain::DurationMinutes::optional(24 * 60 + 10).value(), domain::maxDurationMinutes);
}

TEST(TimeTest, SecondsCannotBeNegative) {
    EXPECT_EQ(domain::ElapsedSeconds::from(-1).value(), 0);
    EXPECT_EQ(domain::RemainingSeconds::from(-1).value(), 0);
}

} // namespace
