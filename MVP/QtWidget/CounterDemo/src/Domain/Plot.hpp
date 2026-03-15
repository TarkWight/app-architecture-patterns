#ifndef PLOT_HPP
#define PLOT_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace domain {

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
};

struct AxisSpec {
    double min{0.0};
    double max{1.0};
    double step{1.0};
    std::string label{};
};

struct PlotModel {
    AxisSpec x{};
    AxisSpec y{};
    Series series{};
    RgbColor color{};
    std::string title{};
};

} // namespace domain

#endif // PLOT_HPP
