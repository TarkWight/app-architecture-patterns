#ifndef PLOTMODEL_HPP
#define PLOTMODEL_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace application::dto {

struct RgbColor {
    std::uint8_t r{255};
    std::uint8_t g{0};
    std::uint8_t b{0};
};

struct Point {
    double x{0.0};
    double y{0.0};
};

struct Series {
    std::vector<Point> points{};
    bool breakOnLargeDelta{false};
    double wrapThreshold{180.0};
};

struct NamedSeries {
    std::string label{};
    Series series{};
    RgbColor color{};
};

struct AxisSpec {
    double min{0.0};
    double max{1.0};
    double step{1.0};
    std::string label{};
    int labelPrecision{-1};
};

struct PlotMarker {
    double x{0.0};
    std::string label{};
    bool visible{false};
};

enum class PlotRenderMode { AnimatedModel, RealtimeTimeSeries };

struct PlotModel {
    AxisSpec x{};
    AxisSpec y{};
    Series series{};
    std::vector<NamedSeries> seriesList{};
    PlotMarker marker{};
    RgbColor color{};
    std::string title{};
    PlotRenderMode renderMode{PlotRenderMode::AnimatedModel};
};

} // namespace application::dto

#endif // PLOTMODEL_HPP
