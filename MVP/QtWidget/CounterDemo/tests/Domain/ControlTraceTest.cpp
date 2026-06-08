#include "../../src/Domain/ControlTrace.hpp"

#include <gtest/gtest.h>

namespace {

TEST(ControlTraceTest, SamplesAlwaysMoveForwardInTime) {
    domain::ControlTrace trace{};
    const auto impact = domain::makeWindProfile(1.0, 0.0, 0.0);

    trace.append(domain::ControlTraceSample{.timeSeconds = 0.0, .targetValue = impact, .safeCommandValue = impact});
    trace.append(domain::ControlTraceSample{.timeSeconds = 0.0, .targetValue = impact, .safeCommandValue = impact});
    trace.append(domain::ControlTraceSample{.timeSeconds = 0.0, .targetValue = impact, .safeCommandValue = impact});

    ASSERT_EQ(trace.size(), 3U);
    EXPECT_DOUBLE_EQ(trace.at(0).timeSeconds, 0.0);
    EXPECT_NEAR(trace.at(1).timeSeconds, domain::ControlTrace::minimumStepSeconds, 0.0001);
    EXPECT_NEAR(trace.at(2).timeSeconds, domain::ControlTrace::minimumStepSeconds * 2.0, 0.0001);
    EXPECT_GT(trace.at(1).timeSeconds, trace.at(0).timeSeconds);
    EXPECT_GT(trace.at(2).timeSeconds, trace.at(1).timeSeconds);
}

TEST(ControlTraceTest, KeepsOnlyRecentSamplesInsideHistoryLimit) {
    domain::ControlTrace trace{};
    const auto impact = domain::makeWindProfile(1.0, 0.0, 0.0);

    for (std::size_t index = 0; index < domain::ControlTrace::maxSamples + 2U; ++index) {
        trace.append(domain::ControlTraceSample{
            .timeSeconds = static_cast<double>(index), .targetValue = impact, .safeCommandValue = impact});
    }

    ASSERT_EQ(trace.size(), domain::ControlTrace::maxSamples);
    EXPECT_DOUBLE_EQ(trace.front().timeSeconds, 2.0);
    EXPECT_DOUBLE_EQ(trace.back().timeSeconds, static_cast<double>(domain::ControlTrace::maxSamples + 1U));
}

} // namespace
