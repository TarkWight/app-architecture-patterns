#include "../../src/Application/Session/SessionState.hpp"

#include <gtest/gtest.h>

namespace {

TEST(SessionStateTest, InitializesTelemetryPlotWithVisibleCoordinateGrid) {
    const application::session::SessionState state{};
    const auto &plot = state.get().telemetry.telemetryPlot;

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

TEST(SessionStateTest, ExposesNarrowStateGroups) {
    application::session::SessionState state{};

    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Connected);
    state.setTelemetryStatus(domain::TelemetryStatus::Valid);
    state.setWindImpact(domain::makeWindImpact(3.0, 90.0, 15.0));
    state.setTestProtocolTitle("Protocol");

    EXPECT_EQ(state.execution().testExecutionStatus, domain::TestExecutionStatus::Running);
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Connected);
    EXPECT_EQ(state.telemetry().telemetryStatus, domain::TelemetryStatus::Valid);
    EXPECT_DOUBLE_EQ(state.control().windImpact.beaufort.value(), 3.0);
    EXPECT_EQ(state.protocol().testProtocol.title, "Protocol");
}

} // namespace
