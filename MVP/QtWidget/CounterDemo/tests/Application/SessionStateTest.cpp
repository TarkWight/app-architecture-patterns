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

} // namespace
