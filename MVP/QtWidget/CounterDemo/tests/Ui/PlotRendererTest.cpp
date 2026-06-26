#include "../../src/Ui/Render/PlotRenderer.hpp"

#include <QApplication>
#include <QImage>
#include <QPainter>

#include <gtest/gtest.h>

#include <cmath>

namespace {

constexpr int rightMargin = 20;

int qtArgc = 1;
char qtArg0[] = "plot-renderer-test";
char *qtArgv[] = {qtArg0, nullptr};
QApplication qtApplication{qtArgc, qtArgv};

TEST(PlotRendererTest, CyclicSeriesBreaksAtLargeDelta) {
    application::dto::Series series{};
    series.breakOnLargeDelta = true;
    series.wrapThreshold = 180.0;
    series.points = {
        application::dto::Point{.x = 0.0, .y = 358.0},
        application::dto::Point{.x = 1.0, .y = 359.0},
        application::dto::Point{.x = 2.0, .y = 0.0},
        application::dto::Point{.x = 3.0, .y = 1.0},
    };

    const auto segments = ui::render::PlotRenderer::splitDrawableSegments(series);

    ASSERT_EQ(segments.size(), 2U);
    ASSERT_EQ(segments.at(0).points.size(), 2U);
    ASSERT_EQ(segments.at(1).points.size(), 2U);
    EXPECT_DOUBLE_EQ(segments.at(0).points.back().y, 359.0);
    EXPECT_DOUBLE_EQ(segments.at(1).points.front().y, 0.0);
}

TEST(PlotRendererTest, NonCyclicSeriesKeepsContinuousLine) {
    application::dto::Series series{};
    series.points = {
        application::dto::Point{.x = 0.0, .y = 358.0},
        application::dto::Point{.x = 1.0, .y = 0.0},
    };

    const auto segments = ui::render::PlotRenderer::splitDrawableSegments(series);

    ASSERT_EQ(segments.size(), 1U);
    ASSERT_EQ(segments.front().points.size(), 2U);
}

TEST(PlotRendererTest, ClipsSeriesPointsOutsideRightPlotBoundary) {
    application::dto::PlotModel plot{};
    plot.x = application::dto::AxisSpec{.min = 0.0, .max = 10.0, .step = 1.0, .label = "seconds"};
    plot.y = application::dto::AxisSpec{.min = 0.0, .max = 10.0, .step = 1.0, .label = "degrees"};
    plot.color = application::dto::RgbColor{255, 0, 0};
    plot.series.points = {application::dto::Point{.x = 10.25, .y = 5.0}};

    QImage image{400, 300, QImage::Format_ARGB32};
    image.fill(Qt::white);

    QPainter painter{&image};
    ui::render::PlotRenderer::drawPlot(painter, QRect{0, 0, image.width(), image.height()}, plot);
    painter.end();

    const int plotRight = image.width() - rightMargin;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = plotRight + 1; x < image.width(); ++x) {
            const QColor color = image.pixelColor(x, y);
            EXPECT_FALSE(color.red() > 200 && color.green() < 80 && color.blue() < 80)
                << "Unexpected series pixel outside plot area at " << x << "," << y;
        }
    }
}

TEST(PlotRendererTest, SelectsReadableXLabelStepForLongControlPlot) {
    const double labelStep = ui::render::PlotRenderer::selectLabelStep(ui::render::AxisLabelStepRequest{
        .minValue = 0.0, .maxValue = 60.0, .gridStep = 1.0, .axisPixels = 720, .minimumLabelSpacingPixels = 48});

    EXPECT_GE(labelStep, 5.0);
    EXPECT_DOUBLE_EQ(std::fmod(labelStep, 1.0), 0.0);
}

TEST(PlotRendererTest, SelectsWholeBeaufortLabelsWhenGridUsesHalfSteps) {
    const double labelStep = ui::render::PlotRenderer::selectLabelStep(ui::render::AxisLabelStepRequest{
        .minValue = 0.0, .maxValue = 7.0, .gridStep = 0.5, .axisPixels = 365, .minimumLabelSpacingPixels = 28});

    EXPECT_DOUBLE_EQ(labelStep, 1.0);
}

TEST(PlotRendererTest, KeepsTelemetrySecondLabelsReadable) {
    const double labelStep = ui::render::PlotRenderer::selectLabelStep(ui::render::AxisLabelStepRequest{
        .minValue = 0.0, .maxValue = 60.0, .gridStep = 10.0, .axisPixels = 720, .minimumLabelSpacingPixels = 48});

    EXPECT_DOUBLE_EQ(labelStep, 10.0);
}

} // namespace
