#include "PlotRenderer.hpp"

#include <QColor>
#include <QPen>
#include <QString>

#include <algorithm>
#include <cmath>
#include <utility>

namespace ui::render {
namespace {

constexpr int leftMargin = 50;
constexpr int rightMargin = 20;
constexpr int topMargin = 20;
constexpr int bottomMargin = 45;
constexpr int minimumXLabelSpacingPixels = 48;
constexpr int minimumYLabelSpacingPixels = 28;

QRect makeInnerPlotRect(const QRect &outerRect) {
    return QRect(outerRect.left() + leftMargin, outerRect.top() + topMargin,
                 outerRect.width() - leftMargin - rightMargin, outerRect.height() - topMargin - bottomMargin);
}

} // namespace

void PlotRenderer::drawPlot(QPainter &painter, const QRect &rect, const application::dto::PlotModel &plot) {
    application::dto::PlotModel drawablePlot = plot;
    if (drawablePlot.x.max <= drawablePlot.x.min) {
        drawablePlot.x = application::dto::AxisSpec{.min = 0.0, .max = 1.0, .step = 0.25, .label = plot.x.label};
    }
    if (drawablePlot.y.max <= drawablePlot.y.min) {
        drawablePlot.y = application::dto::AxisSpec{.min = 0.0, .max = 1.0, .step = 0.25, .label = plot.y.label};
    }

    const QRect plotRect = makeInnerPlotRect(rect);

    drawFrame(painter, plotRect);
    drawTitle(painter, rect, drawablePlot);
    drawAxisLabels(painter, plotRect, drawablePlot);
    drawXGrid(painter, plotRect, drawablePlot, leftMargin);
    drawYGrid(painter, plotRect, drawablePlot, leftMargin);

    if (!hasRenderablePlot(drawablePlot)) {
        drawMarker(painter, plotRect, drawablePlot);
        return;
    }

    painter.save();
    painter.setClipRect(plotRect);
    drawSeries(painter, plotRect, drawablePlot);
    painter.restore();

    drawLegend(painter, plotRect, drawablePlot);
    drawMarker(painter, plotRect, drawablePlot);
}

bool PlotRenderer::hasRenderablePlot(const application::dto::PlotModel &plot) {
    if (plot.x.max <= plot.x.min || plot.y.max <= plot.y.min) {
        return false;
    }

    if (!plot.series.points.empty()) {
        return true;
    }

    return std::any_of(plot.seriesList.begin(), plot.seriesList.end(),
                       [](const application::dto::NamedSeries &series) { return !series.series.points.empty(); });
}

std::vector<application::dto::Series> PlotRenderer::splitDrawableSegments(const application::dto::Series &series) {
    if (!series.breakOnLargeDelta || series.points.size() < 2) {
        return {series};
    }

    std::vector<application::dto::Series> segments{};
    application::dto::Series current{};
    current.breakOnLargeDelta = series.breakOnLargeDelta;
    current.wrapThreshold = series.wrapThreshold;

    for (const auto &point : series.points) {
        if (!current.points.empty() && std::abs(point.y - current.points.back().y) > series.wrapThreshold) {
            segments.push_back(std::move(current));
            current = application::dto::Series{};
            current.breakOnLargeDelta = series.breakOnLargeDelta;
            current.wrapThreshold = series.wrapThreshold;
        }

        current.points.push_back(point);
    }

    if (!current.points.empty()) {
        segments.push_back(std::move(current));
    }

    return segments;
}

double PlotRenderer::normalize(double value, double min, double max) {
    if (max <= min) {
        return 0.0;
    }

    return (value - min) / (max - min);
}

QColor PlotRenderer::toQColor(application::dto::RgbColor color) {
    return QColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b));
}

int PlotRenderer::projectX(const QRect &plotRect, const application::dto::PlotModel &plot, double xValue) {
    const double ratio = normalize(xValue, plot.x.min, plot.x.max);
    return plotRect.left() + static_cast<int>(std::lround(ratio * static_cast<double>(plotRect.width())));
}

int PlotRenderer::projectY(const QRect &plotRect, const application::dto::PlotModel &plot, double yValue) {
    const double ratio = normalize(yValue, plot.y.min, plot.y.max);
    return plotRect.bottom() - static_cast<int>(std::lround(ratio * static_cast<double>(plotRect.height())));
}

int PlotRenderer::tickCount(double minValue, double maxValue, double stepValue) {
    if (stepValue <= 0.0 || maxValue <= minValue) {
        return 0;
    }

    const double span = maxValue - minValue;
    return static_cast<int>(std::floor(span / stepValue)) + 1;
}

double niceStep(double rawStep) {
    if (!std::isfinite(rawStep) || rawStep <= 0.0) {
        return 1.0;
    }

    const double exponent = std::floor(std::log10(rawStep));
    const double scale = std::pow(10.0, exponent);
    const double normalized = rawStep / scale;

    if (normalized <= 1.0) {
        return scale;
    }
    if (normalized <= 2.0) {
        return 2.0 * scale;
    }
    if (normalized <= 5.0) {
        return 5.0 * scale;
    }

    return 10.0 * scale;
}

int automaticPrecision(double step) {
    if (std::abs(step - std::round(step)) < 0.000001) {
        return 0;
    }

    if (std::abs((step * 10.0) - std::round(step * 10.0)) < 0.000001) {
        return 1;
    }

    return 2;
}

bool isLabelTick(double value, double minValue, double labelStep) {
    if (labelStep <= 0.0) {
        return true;
    }

    const double tickIndex = (value - minValue) / labelStep;
    return std::abs(tickIndex - std::round(tickIndex)) < 0.000001;
}

QString PlotRenderer::formatTickValue(double value, const application::dto::AxisSpec &axis, double labelStep) {
    if (axis.labelPrecision >= 0) {
        return QString::number(value, 'f', axis.labelPrecision);
    }

    return QString::number(value, 'f', automaticPrecision(labelStep));
}

double PlotRenderer::selectLabelStep(const AxisLabelStepRequest &request) {
    if (request.gridStep <= 0.0 || request.maxValue <= request.minValue || request.axisPixels <= 0 ||
        request.minimumLabelSpacingPixels <= 0) {
        return request.gridStep;
    }

    const double span = request.maxValue - request.minValue;
    const double maximumLabelCount = std::max(1.0, std::floor(static_cast<double>(request.axisPixels) /
                                                              static_cast<double>(request.minimumLabelSpacingPixels)));
    const double desiredStep = niceStep(span / maximumLabelCount);
    const double multiplier = std::max(1.0, std::ceil((desiredStep / request.gridStep) - 0.000001));

    return request.gridStep * multiplier;
}

void PlotRenderer::drawFrame(QPainter &painter, const QRect &plotRect) {
    painter.fillRect(plotRect, QColor(255, 255, 255));
    painter.setPen(QPen(QColor(30, 41, 59), 1));
    painter.drawRect(plotRect);
}

void PlotRenderer::drawTitle(QPainter &painter, const QRect &outerRect, const application::dto::PlotModel &plot) {
    if (plot.title.empty()) {
        return;
    }

    painter.drawText(QRect(outerRect.left(), outerRect.top(), outerRect.width(), topMargin), Qt::AlignCenter,
                     QString::fromStdString(plot.title));
}

void PlotRenderer::drawAxisLabels(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot) {
    painter.drawText(QRect(plotRect.left(), plotRect.bottom() + 8, plotRect.width(), 20), Qt::AlignCenter,
                     QString::fromStdString(plot.x.label));

    painter.save();
    painter.translate(static_cast<double>(plotRect.left()) + 15.0,
                      static_cast<double>(plotRect.top()) + (static_cast<double>(plotRect.height()) / 2.0));
    painter.rotate(-90.0);
    painter.drawText(QRect(-plotRect.height() / 2, -20, plotRect.height(), 20), Qt::AlignCenter,
                     QString::fromStdString(plot.y.label));
    painter.restore();
}

void PlotRenderer::drawXGrid(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot,
                             int /*leftMargin*/
) {
    const int count = tickCount(plot.x.min, plot.x.max, plot.x.step);
    if (count <= 0) {
        return;
    }

    const double labelStep =
        selectLabelStep(AxisLabelStepRequest{.minValue = plot.x.min,
                                             .maxValue = plot.x.max,
                                             .gridStep = plot.x.step,
                                             .axisPixels = plotRect.width(),
                                             .minimumLabelSpacingPixels = minimumXLabelSpacingPixels});

    painter.setPen(QPen(QColor(203, 213, 225), 1, Qt::DashLine));

    for (int index = 0; index < count; ++index) {
        const double xValue = plot.x.min + (static_cast<double>(index) * plot.x.step);
        const int px = projectX(plotRect, plot, xValue);

        painter.drawLine(px, plotRect.top(), px, plotRect.bottom());

        if (!isLabelTick(xValue, plot.x.min, labelStep)) {
            continue;
        }

        painter.setPen(QPen(QColor(30, 41, 59), 1));
        painter.drawText(px - 24, plotRect.bottom() + 18, 48, 16, Qt::AlignCenter,
                         formatTickValue(xValue, plot.x, labelStep));
        painter.setPen(QPen(QColor(203, 213, 225), 1, Qt::DashLine));
    }
}

void PlotRenderer::drawYGrid(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot,
                             int leftMargin) {
    const int count = tickCount(plot.y.min, plot.y.max, plot.y.step);
    if (count <= 0) {
        return;
    }

    const double labelStep =
        selectLabelStep(AxisLabelStepRequest{.minValue = plot.y.min,
                                             .maxValue = plot.y.max,
                                             .gridStep = plot.y.step,
                                             .axisPixels = plotRect.height(),
                                             .minimumLabelSpacingPixels = minimumYLabelSpacingPixels});

    painter.setPen(QPen(QColor(203, 213, 225), 1, Qt::DashLine));

    for (int index = 0; index < count; ++index) {
        const double yValue = plot.y.min + (static_cast<double>(index) * plot.y.step);
        const int py = projectY(plotRect, plot, yValue);

        painter.drawLine(plotRect.left(), py, plotRect.right(), py);

        if (!isLabelTick(yValue, plot.y.min, labelStep)) {
            continue;
        }

        painter.setPen(QPen(QColor(30, 41, 59), 1));
        painter.drawText(plotRect.left() - leftMargin, py - 8, leftMargin - 5, 16, Qt::AlignRight | Qt::AlignVCenter,
                         formatTickValue(yValue, plot.y, labelStep));
        painter.setPen(QPen(QColor(203, 213, 225), 1, Qt::DashLine));
    }
}

void PlotRenderer::drawMarker(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot) {
    if (!plot.marker.visible || plot.marker.x < plot.x.min || plot.marker.x > plot.x.max) {
        return;
    }

    const int px = projectX(plotRect, plot, plot.marker.x);
    painter.setPen(QPen(QColor(71, 85, 105), 2, Qt::DashDotLine));
    painter.drawLine(px, plotRect.top(), px, plotRect.bottom());

    if (!plot.marker.label.empty()) {
        painter.setPen(QPen(QColor(30, 41, 59), 1));
        painter.drawText(px + 4, plotRect.top() + 4, 120, 18, Qt::AlignLeft | Qt::AlignVCenter,
                         QString::fromStdString(plot.marker.label));
    }
}

QPolygon PlotRenderer::buildPolyline(const QRect &plotRect, const application::dto::PlotModel &plot,
                                     const application::dto::Series &series) {
    QPolygon polyline;
    polyline.reserve(static_cast<int>(series.points.size()));

    for (const auto &point : series.points) {
        polyline << QPoint(projectX(plotRect, plot, point.x), projectY(plotRect, plot, point.y));
    }

    return polyline;
}

void PlotRenderer::drawPoint(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot,
                             const application::dto::Point &point) {
    constexpr int radius = 4;
    const QPoint center{projectX(plotRect, plot, point.x), projectY(plotRect, plot, point.y)};
    painter.drawEllipse(center, radius, radius);
}

void PlotRenderer::drawSeries(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot) {
    if (!plot.seriesList.empty()) {
        for (const auto &series : plot.seriesList) {
            if (series.series.points.empty()) {
                continue;
            }

            painter.setPen(QPen(toQColor(series.color), 2));
            if (series.series.points.size() == 1) {
                drawPoint(painter, plotRect, plot, series.series.points.front());
            } else {
                for (const auto &segment : splitDrawableSegments(series.series)) {
                    if (segment.points.size() == 1) {
                        drawPoint(painter, plotRect, plot, segment.points.front());
                    } else {
                        const QPolygon polyline = buildPolyline(plotRect, plot, segment);
                        painter.drawPolyline(polyline);
                    }
                }
            }
        }

        return;
    }

    painter.setPen(QPen(toQColor(plot.color), 2));
    if (plot.series.points.size() == 1) {
        drawPoint(painter, plotRect, plot, plot.series.points.front());
    } else {
        for (const auto &segment : splitDrawableSegments(plot.series)) {
            if (segment.points.size() == 1) {
                drawPoint(painter, plotRect, plot, segment.points.front());
            } else {
                const QPolygon polyline = buildPolyline(plotRect, plot, segment);
                painter.drawPolyline(polyline);
            }
        }
    }
}

void PlotRenderer::drawLegend(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot) {
    int y = plotRect.top() + 8;
    const int x = plotRect.right() - 170;

    const int visibleLabels = static_cast<int>(
        std::count_if(plot.seriesList.begin(), plot.seriesList.end(),
                      [](const application::dto::NamedSeries &series) { return !series.label.empty(); }));
    if (visibleLabels > 0) {
        painter.fillRect(QRect(x - 8, y - 5, 170, (visibleLabels * 20) + 10), QColor(255, 255, 255, 230));
        painter.setPen(QPen(QColor(226, 232, 240), 1));
        painter.drawRect(QRect(x - 8, y - 5, 170, (visibleLabels * 20) + 10));
    }

    painter.setPen(QPen(QColor(30, 41, 59), 1));

    for (const auto &series : plot.seriesList) {
        if (series.label.empty()) {
            continue;
        }

        painter.setPen(QPen(toQColor(series.color), 3));
        painter.drawLine(x, y + 8, x + 22, y + 8);

        painter.setPen(QPen(QColor(30, 41, 59), 1));
        painter.drawText(x + 28, y, 140, 18, Qt::AlignLeft | Qt::AlignVCenter, QString::fromStdString(series.label));

        y += 20;
    }
}

} // namespace ui::render
