#include "../../src/Ui/Render/PlotRenderer.hpp"

#include <gtest/gtest.h>

namespace {

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

} // namespace
