#include "../../src/Application/Services/TelemetryPlotBuilder.hpp"

#include "../../src/Domain/AxisId.hpp"

#include <gtest/gtest.h>

namespace {

domain::AxisTelemetrySample validSample(domain::AxisId axisId, double timestampSeconds, float position) {
    domain::AxisTelemetrySample sample{};
    sample.axisId = axisId;
    sample.timestampSeconds = timestampSeconds;
    sample.position = position;
    sample.valid = true;
    return sample;
}

TEST(TelemetryPlotBuilderTest, BuildsVisibleGridWhenHistoryIsEmpty) {
    const application::session::TelemetryStateData telemetry{};
    const application::services::TelemetryPlotBuilder builder{};

    const auto plot = builder.build(telemetry);

    EXPECT_EQ(plot.title, "Telemetry");
    EXPECT_DOUBLE_EQ(plot.x.min, 0.0);
    EXPECT_DOUBLE_EQ(plot.x.max, 60.0);
    EXPECT_DOUBLE_EQ(plot.x.step, 10.0);
    EXPECT_EQ(plot.x.label, "seconds");
    EXPECT_DOUBLE_EQ(plot.y.min, -180.0);
    EXPECT_DOUBLE_EQ(plot.y.max, 360.0);
    EXPECT_DOUBLE_EQ(plot.y.step, 45.0);
    EXPECT_EQ(plot.y.label, "degrees");
    ASSERT_EQ(plot.seriesList.size(), 2U);
    EXPECT_EQ(plot.seriesList.at(0).label, "Ось Y / тангаж");
    EXPECT_EQ(plot.seriesList.at(1).label, "Ось Z / направление");
}

TEST(TelemetryPlotBuilderTest, BuildsAxisSeriesInsideSelectedWindow) {
    application::session::TelemetryStateData telemetry{};
    telemetry.telemetryWindowEndSeconds = domain::TelemetryWindowEnd::fromSeconds(70.0);
    telemetry.telemetryHistory.push_back(validSample(domain::axis0, 100.0, 1.0F));
    telemetry.telemetryHistory.push_back(validSample(domain::axis1, 120.0, 2.0F));
    telemetry.telemetryHistory.push_back(validSample(domain::axis0, 160.0, 3.0F));
    telemetry.telemetryHistory.push_back(validSample(domain::axis1, 180.0, 4.0F));
    const application::services::TelemetryPlotBuilder builder{};

    const auto plot = builder.build(telemetry);

    EXPECT_DOUBLE_EQ(plot.x.min, 10.0);
    EXPECT_DOUBLE_EQ(plot.x.max, 70.0);
    ASSERT_EQ(plot.seriesList.size(), 2U);
    ASSERT_EQ(plot.seriesList.at(0).series.points.size(), 1U);
    ASSERT_EQ(plot.seriesList.at(1).series.points.size(), 1U);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(0).series.points.at(0).x, 60.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(0).series.points.at(0).y, 3.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(1).series.points.at(0).x, 20.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(1).series.points.at(0).y, 2.0);
}

TEST(TelemetryPlotBuilderTest, AppliesAxisVisibilityAndColors) {
    application::session::TelemetryStateData telemetry{};
    telemetry.telemetryAxisYVisible = false;
    telemetry.telemetryAxisZColor = application::dto::RgbColor{1, 2, 3};
    telemetry.telemetryHistory.push_back(validSample(domain::axis0, 10.0, 1.0F));
    telemetry.telemetryHistory.push_back(validSample(domain::axis1, 20.0, 2.0F));
    const application::services::TelemetryPlotBuilder builder{};

    const auto plot = builder.build(telemetry);

    ASSERT_EQ(plot.seriesList.size(), 1U);
    EXPECT_EQ(plot.seriesList.at(0).label, "Ось Z / направление");
    EXPECT_EQ(plot.seriesList.at(0).color.r, 1);
    EXPECT_EQ(plot.seriesList.at(0).color.g, 2);
    EXPECT_EQ(plot.seriesList.at(0).color.b, 3);
    ASSERT_EQ(plot.seriesList.at(0).series.points.size(), 1U);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(0).series.points.at(0).y, 2.0);
}

} // namespace
