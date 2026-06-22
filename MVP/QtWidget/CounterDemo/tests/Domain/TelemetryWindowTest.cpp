#include "../../src/Domain/TelemetryWindow.hpp"

#include <gtest/gtest.h>

namespace {

TEST(TelemetryWindowEndTest, ClampsNegativeSecondsToZero) {
    const auto end = domain::TelemetryWindowEnd::fromSeconds(-12.5);

    EXPECT_DOUBLE_EQ(end.seconds(), 0.0);
}

TEST(TelemetryWindowEndTest, KeepsPositiveSeconds) {
    const auto end = domain::TelemetryWindowEnd::fromSeconds(42.5);

    EXPECT_DOUBLE_EQ(end.seconds(), 42.5);
}

TEST(TelemetryWindowEndTest, CalculatesTailEndFromTelemetrySampleRange) {
    domain::AxisTelemetrySample first{};
    first.timestampSeconds = 10.0;
    domain::AxisTelemetrySample last{};
    last.timestampSeconds = 42.5;

    const auto end = domain::TelemetryWindowEnd::fromTail(first, last);

    EXPECT_DOUBLE_EQ(end.seconds(), 32.5);
}

} // namespace
