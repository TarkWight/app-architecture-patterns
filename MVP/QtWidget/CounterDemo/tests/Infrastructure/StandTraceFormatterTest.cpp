#include "../../src/Infrastructure/AxisTcp/StandTraceFormatter.hpp"

#include <gtest/gtest.h>

namespace {

TEST(StandTraceFormatterTest, FormatsCommandSentTrace) {
    const auto message =
        infrastructure::axisTcp::formatStandTxTrace(domain::axis0, domain::AxisControlCommand{.position = 12.5F,
                                                                                              .velocity = 0.06F,
                                                                                              .torque = 4.55F,
                                                                                              .cmd1 = true,
                                                                                              .cmd2 = true,
                                                                                              .cmd3 = false,
                                                                                              .cmd4 = true});

    EXPECT_EQ(message, "[STAND][TX][axis0] pos=12.500, vel=0.060, torque=4.550, flags=(1,1,0,1)");
}

TEST(StandTraceFormatterTest, FormatsTelemetryReceivedTrace) {
    domain::AxisTelemetrySample sample{};
    sample.axisId = domain::axis1;
    sample.position = 359.5F;
    sample.setPosition = 0.5F;
    sample.torque = 3.5F;
    sample.voltage = 28.3F;
    sample.current = 1.25F;

    const auto message = infrastructure::axisTcp::formatStandRxTrace(sample);

    EXPECT_EQ(message, "[STAND][RX][axis1] pos=359.500, setPos=0.500, torque=3.500, voltage=28.300, ibus=1.250");
}

TEST(StandTraceFormatterTest, FormatsLifecycleTrace) {
    const auto message = infrastructure::axisTcp::formatStandLifecycleTrace("CONNECTED", domain::axis1);

    EXPECT_EQ(message, "[STAND][CONNECTED][axis1]");
}

} // namespace
