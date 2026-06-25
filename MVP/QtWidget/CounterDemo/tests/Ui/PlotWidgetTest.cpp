#include "../../src/Ui/PlotWidget.hpp"

#include <gtest/gtest.h>

namespace {

application::dto::PlotModel realtimePlot(double xMin, double xMax, double firstY) {
    application::dto::PlotModel plot{};
    plot.renderMode = application::dto::PlotRenderMode::RealtimeTimeSeries;
    plot.x = application::dto::AxisSpec{.min = xMin, .max = xMax, .step = 10.0, .label = "seconds"};

    application::dto::NamedSeries series{};
    series.label = "telemetry";
    series.series.points = {
        application::dto::Point{.x = xMax - 1.0, .y = firstY},
        application::dto::Point{.x = xMax, .y = firstY + 1.0},
    };
    plot.seriesList.push_back(series);
    return plot;
}

TEST(PlotWidgetTest, RealtimeUpdateKeepsPreviousViewportAndUsesTargetSeries) {
    const auto oldPlot = realtimePlot(0.0, 60.0, 10.0);
    const auto newPlot = realtimePlot(1.0, 61.0, 20.0);

    const auto prepared = ui::PlotWidget::prepareRealtimePlotForAnimation(oldPlot, newPlot);

    EXPECT_DOUBLE_EQ(prepared.x.min, oldPlot.x.min);
    EXPECT_DOUBLE_EQ(prepared.x.max, oldPlot.x.max);
    ASSERT_EQ(prepared.seriesList.size(), 1U);
    ASSERT_EQ(prepared.seriesList.front().series.points.size(), 2U);
    EXPECT_DOUBLE_EQ(prepared.seriesList.front().series.points.front().y, 20.0);
    EXPECT_DOUBLE_EQ(prepared.seriesList.front().series.points.back().y, 21.0);
}

TEST(PlotWidgetTest, RealtimeInterpolationMovesOnlyViewport) {
    const auto oldPlot = realtimePlot(0.0, 60.0, 10.0);
    const auto newPlot = realtimePlot(1.0, 61.0, 20.0);
    const auto prepared = ui::PlotWidget::prepareRealtimePlotForAnimation(oldPlot, newPlot);

    const auto frame = ui::PlotWidget::interpolateRealtimeViewport(prepared, newPlot);

    EXPECT_GT(frame.x.min, oldPlot.x.min);
    EXPECT_LT(frame.x.min, newPlot.x.min);
    EXPECT_GT(frame.x.max, oldPlot.x.max);
    EXPECT_LT(frame.x.max, newPlot.x.max);
    ASSERT_EQ(frame.seriesList.size(), 1U);
    ASSERT_EQ(frame.seriesList.front().series.points.size(), 2U);
    EXPECT_DOUBLE_EQ(frame.seriesList.front().series.points.front().y, 20.0);
    EXPECT_DOUBLE_EQ(frame.seriesList.front().series.points.back().y, 21.0);

    const auto finalFrame = ui::PlotWidget::interpolateRealtimeViewport(newPlot, newPlot);
    EXPECT_DOUBLE_EQ(finalFrame.x.min, newPlot.x.min);
    EXPECT_DOUBLE_EQ(finalFrame.x.max, newPlot.x.max);
}

} // namespace
