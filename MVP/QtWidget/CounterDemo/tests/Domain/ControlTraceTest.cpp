#include "../../src/Domain/ControlTrace.hpp"

#include <gtest/gtest.h>

namespace {

TEST(ControlTraceTimeTest, ClampsNegativeSecondsToZero) {
    const auto time = domain::ControlTraceTime::fromSeconds(-5.0);

    EXPECT_DOUBLE_EQ(time.seconds(), 0.0);
    EXPECT_DOUBLE_EQ(time.minutes(), 0.0);
}

TEST(ControlTraceTest, SamplesAlwaysMoveForwardInTime) {
    domain::ControlTrace trace{};
    const auto impact = domain::makeWindImpact(1.0, 0.0, 0.0);

    trace.append(domain::ControlTraceSample{
        .time = domain::ControlTraceTime::fromSeconds(0.0), .targetValue = impact, .safeCommandValue = impact});
    trace.append(domain::ControlTraceSample{
        .time = domain::ControlTraceTime::fromSeconds(0.0), .targetValue = impact, .safeCommandValue = impact});
    trace.append(domain::ControlTraceSample{
        .time = domain::ControlTraceTime::fromSeconds(0.0), .targetValue = impact, .safeCommandValue = impact});

    ASSERT_EQ(trace.size(), 3U);
    EXPECT_DOUBLE_EQ(trace.at(0).time.seconds(), 0.0);
    EXPECT_NEAR(trace.at(1).time.seconds(), domain::ControlTrace::minimumStepSeconds, 0.0001);
    EXPECT_NEAR(trace.at(2).time.seconds(), domain::ControlTrace::minimumStepSeconds * 2.0, 0.0001);
    EXPECT_GT(trace.at(1).time.seconds(), trace.at(0).time.seconds());
    EXPECT_GT(trace.at(2).time.seconds(), trace.at(1).time.seconds());
}

TEST(ControlTraceTest, KeepsOnlyRecentSamplesInsideHistoryLimit) {
    domain::ControlTrace trace{};
    const auto impact = domain::makeWindImpact(1.0, 0.0, 0.0);

    for (std::size_t index = 0; index < domain::ControlTrace::maxSamples + 2U; ++index) {
        trace.append(
            domain::ControlTraceSample{.time = domain::ControlTraceTime::fromSeconds(static_cast<double>(index)),
                                       .targetValue = impact,
                                       .safeCommandValue = impact});
    }

    ASSERT_EQ(trace.size(), domain::ControlTrace::maxSamples);
    EXPECT_DOUBLE_EQ(trace.front().time.seconds(), 2.0);
    EXPECT_DOUBLE_EQ(trace.back().time.seconds(), static_cast<double>(domain::ControlTrace::maxSamples + 1U));
}

TEST(ControlTraceTest, ManualCommandSampleCapturesElapsedTargetAndSafeCommandValues) {
    const auto target = domain::makeWindImpact(5.0, 90.0, 4.0);
    const auto safeCommand = domain::makeWindImpact(1.2, 20.0, 2.0);

    const auto sample = domain::ControlTraceSample::manualCommand(domain::ElapsedSeconds::from(7), target, safeCommand);

    EXPECT_DOUBLE_EQ(sample.time.seconds(), 7.0);
    EXPECT_DOUBLE_EQ(sample.targetValue.beaufort.value(), 5.0);
    EXPECT_DOUBLE_EQ(sample.targetValue.direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(sample.safeCommandValue.beaufort.value(), 1.2);
    EXPECT_DOUBLE_EQ(sample.safeCommandValue.direction.degrees(), 20.0);
}

} // namespace
