#include "PlotWidget.hpp"

#include "Render/PlotRenderer.hpp"

#include <QPaintEvent>
#include <QPainter>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

namespace ui {

PlotWidget::PlotWidget(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(220);

    animationTimer.setInterval(33);
    QObject::connect(&animationTimer, &QTimer::timeout, this, &PlotWidget::advanceAnimationFrame);
}

void PlotWidget::setPlot(domain::PlotModel plot) {
    targetPlot = std::move(plot);

    if (!animationTimer.isActive() && this->plot.title.empty() && this->plot.series.points.empty() &&
        this->plot.seriesList.empty()) {
        this->plot = targetPlot;
        update();
        return;
    }

    animationTimer.start();
    update();
}

const domain::PlotModel &PlotWidget::getPlot() const {
    return plot;
}

void PlotWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), palette().base());

    ui::render::PlotRenderer::drawPlot(painter, rect(), plot);
}

void PlotWidget::advanceAnimationFrame() {
    plot = interpolatePlot(plot, targetPlot);

    if (isClose(plot, targetPlot)) {
        plot = targetPlot;
        animationTimer.stop();
    }

    update();
}

domain::PlotModel PlotWidget::interpolatePlot(const domain::PlotModel &current, const domain::PlotModel &target) {
    domain::PlotModel result = target;
    result.x = interpolateAxis(current.x, target.x);
    result.y = interpolateAxis(current.y, target.y);
    result.marker.x = interpolateValue(current.marker.x, target.marker.x);
    result.color = interpolateColor(current.color, target.color);
    result.series = interpolateSeries(current.series, target.series);

    if (current.seriesList.size() == target.seriesList.size()) {
        result.seriesList.clear();
        result.seriesList.reserve(target.seriesList.size());

        for (std::size_t index = 0; index < target.seriesList.size(); ++index) {
            domain::NamedSeries series = target.seriesList[index];
            series.color = interpolateColor(current.seriesList[index].color, target.seriesList[index].color);
            series.series = interpolateSeries(current.seriesList[index].series, target.seriesList[index].series);
            result.seriesList.push_back(std::move(series));
        }
    }

    return result;
}

domain::AxisSpec PlotWidget::interpolateAxis(domain::AxisSpec current, domain::AxisSpec target) {
    target.min = interpolateValue(current.min, target.min);
    target.max = interpolateValue(current.max, target.max);
    target.step = interpolateValue(current.step, target.step);
    return target;
}

domain::Series PlotWidget::interpolateSeries(const domain::Series &current, const domain::Series &target) {
    if (current.points.size() != target.points.size()) {
        return target;
    }

    domain::Series result{};
    result.points.reserve(target.points.size());

    for (std::size_t index = 0; index < target.points.size(); ++index) {
        result.points.push_back(domain::Point{.x = interpolateValue(current.points[index].x, target.points[index].x),
                                              .y = interpolateValue(current.points[index].y, target.points[index].y)});
    }

    return result;
}

domain::RgbColor PlotWidget::interpolateColor(domain::RgbColor current, domain::RgbColor target) {
    auto blend = [](std::uint8_t from, std::uint8_t to) {
        return static_cast<std::uint8_t>(std::clamp(
            static_cast<int>(std::lround(interpolateValue(static_cast<double>(from), static_cast<double>(to)))), 0,
            255));
    };

    return domain::RgbColor{
        .r = blend(current.r, target.r), .g = blend(current.g, target.g), .b = blend(current.b, target.b)};
}

double PlotWidget::interpolateValue(double current, double target) {
    constexpr double factor = 0.22;
    return current + ((target - current) * factor);
}

bool PlotWidget::isClose(const domain::PlotModel &current, const domain::PlotModel &target) {
    constexpr double epsilon = 0.01;
    if (!isClose(current.x, target.x) || !isClose(current.y, target.y) ||
        std::abs(current.marker.x - target.marker.x) >= epsilon || !isClose(current.series, target.series) ||
        current.seriesList.size() != target.seriesList.size()) {
        return false;
    }

    for (std::size_t index = 0; index < current.seriesList.size(); ++index) {
        if (!isClose(current.seriesList[index], target.seriesList[index])) {
            return false;
        }
    }

    return true;
}

bool PlotWidget::isClose(const domain::AxisSpec &current, const domain::AxisSpec &target) {
    constexpr double epsilon = 0.01;
    return std::abs(current.min - target.min) < epsilon && std::abs(current.max - target.max) < epsilon &&
           std::abs(current.step - target.step) < epsilon;
}

bool PlotWidget::isClose(const domain::Series &current, const domain::Series &target) {
    constexpr double epsilon = 0.01;
    if (current.points.size() != target.points.size()) {
        return false;
    }

    for (std::size_t index = 0; index < current.points.size(); ++index) {
        if (std::abs(current.points[index].x - target.points[index].x) >= epsilon ||
            std::abs(current.points[index].y - target.points[index].y) >= epsilon) {
            return false;
        }
    }

    return true;
}

bool PlotWidget::isClose(const domain::NamedSeries &current, const domain::NamedSeries &target) {
    return isClose(current.series, target.series);
}

} // namespace ui
