#include "../../src/Domain/TelemetryPollInterval.hpp"

#include <gtest/gtest.h>

namespace {

TEST(TelemetryPollIntervalTest, ClampsIntervalBelowMinimum) {
    const auto interval = domain::TelemetryPollInterval::fromMilliseconds(1);

    EXPECT_EQ(interval.milliseconds(), domain::minTelemetryPollIntervalMs);
}

TEST(TelemetryPollIntervalTest, KeepsIntervalInsideOperationalRange) {
    const auto interval = domain::TelemetryPollInterval::fromMilliseconds(250);

    EXPECT_EQ(interval.milliseconds(), 250);
}

TEST(TelemetryPollIntervalTest, ClampsIntervalAboveMaximum) {
    const auto interval = domain::TelemetryPollInterval::fromMilliseconds(120'000);

    EXPECT_EQ(interval.milliseconds(), domain::maxTelemetryPollIntervalMs);
}

} // namespace
