#include "../../src/Application/Session/SessionState.hpp"

#include <gtest/gtest.h>

namespace {

TEST(SessionStateTest, InitializesTelemetryPlotWithVisibleCoordinateGrid) {
    const application::session::SessionState state{};
    const auto &plot = state.get().telemetryPlot;

    EXPECT_EQ(plot.title, "Telemetry");
    EXPECT_DOUBLE_EQ(plot.x.min, 0.0);
    EXPECT_DOUBLE_EQ(plot.x.max, 60.0);
    EXPECT_DOUBLE_EQ(plot.x.step, 10.0);
    EXPECT_EQ(plot.x.label, "seconds");
    EXPECT_DOUBLE_EQ(plot.y.min, -180.0);
    EXPECT_DOUBLE_EQ(plot.y.max, 360.0);
    EXPECT_DOUBLE_EQ(plot.y.step, 45.0);
    EXPECT_EQ(plot.y.label, "degrees");
}

TEST(SessionStateTest, ControlTraceSamplesAlwaysMoveForwardInTime) {
    application::session::SessionState state{};
    const auto impact = domain::makeWindProfile(1.0, 0.0, 0.0);

    state.appendControlTraceSample(
        domain::ControlTraceSample{.timeSeconds = 0.0, .targetValue = impact, .safeCommandValue = impact});
    state.appendControlTraceSample(
        domain::ControlTraceSample{.timeSeconds = 0.0, .targetValue = impact, .safeCommandValue = impact});
    state.appendControlTraceSample(
        domain::ControlTraceSample{.timeSeconds = 0.0, .targetValue = impact, .safeCommandValue = impact});

    ASSERT_EQ(state.get().controlTraceHistory.size(), 3U);
    EXPECT_DOUBLE_EQ(state.get().controlTraceHistory.at(0).timeSeconds, 0.0);
    EXPECT_NEAR(state.get().controlTraceHistory.at(1).timeSeconds, 0.1, 0.0001);
    EXPECT_NEAR(state.get().controlTraceHistory.at(2).timeSeconds, 0.2, 0.0001);
    EXPECT_GT(state.get().controlTraceHistory.at(1).timeSeconds, state.get().controlTraceHistory.at(0).timeSeconds);
    EXPECT_GT(state.get().controlTraceHistory.at(2).timeSeconds, state.get().controlTraceHistory.at(1).timeSeconds);
}

} // namespace
