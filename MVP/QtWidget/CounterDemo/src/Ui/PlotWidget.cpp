#include "PlotWidget.hpp"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QString>

#include <cmath>

namespace ui {
namespace {

constexpr int leftMargin = 50;
constexpr int rightMargin = 20;
constexpr int topMargin = 20;
constexpr int bottomMargin = 45;

QRect makePlotRect(const QWidget *widget) {
    return QRect(leftMargin, topMargin, widget->width() - leftMargin - rightMargin,
                 widget->height() - topMargin - bottomMargin);
}

QColor toQColor(domain::RgbColor color) {
    return QColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b));
}

int projectX(const QRect &plotRect, const domain::PlotModel &plot, double xValue) {
    const double ratio = PlotWidget::normalize(xValue, plot.x.min, plot.x.max);
    return plotRect.left() + static_cast<int>(std::lround(ratio * static_cast<double>(plotRect.width())));
}

int projectY(const QRect &plotRect, const domain::PlotModel &plot, double yValue) {
    const double ratio = PlotWidget::normalize(yValue, plot.y.min, plot.y.max);
    return plotRect.bottom() - static_cast<int>(std::lround(ratio * static_cast<double>(plotRect.height())));
}

int tickCount(double minValue, double maxValue, double stepValue) {
    if (stepValue <= 0.0 || maxValue <= minValue) {
        return 0;
    }

    const double span = maxValue - minValue;
    return static_cast<int>(std::floor(span / stepValue)) + 1;
}

void drawBackground(QPainter &painter, const QWidget *widget) {
    painter.fillRect(widget->rect(), widget->palette().base());
}

void drawFrame(QPainter &painter, const QRect &plotRect) {
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(plotRect);
}

void drawTitle(QPainter &painter, const QWidget *widget, const domain::PlotModel &plot) {
    if (plot.title.empty()) {
        return;
    }

    painter.drawText(QRect(0, 0, widget->width(), topMargin), Qt::AlignCenter, QString::fromStdString(plot.title));
}

void drawAxisLabels(QPainter &painter, const QRect &plotRect, const domain::PlotModel &plot) {
    painter.drawText(QRect(plotRect.left(), plotRect.bottom() + 8, plotRect.width(), 20), Qt::AlignCenter,
                     QString::fromStdString(plot.x.label));

    painter.save();
    painter.translate(15.0, static_cast<double>(plotRect.top()) + (static_cast<double>(plotRect.height()) / 2.0));
    painter.rotate(-90.0);
    painter.drawText(QRect(-plotRect.height() / 2, -20, plotRect.height(), 20), Qt::AlignCenter,
                     QString::fromStdString(plot.y.label));
    painter.restore();
}

void drawXGrid(QPainter &painter, const QRect &plotRect, const domain::PlotModel &plot) {
    const int count = tickCount(plot.x.min, plot.x.max, plot.x.step);
    if (count <= 0) {
        return;
    }

    painter.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));

    for (int index = 0; index < count; ++index) {
        const double xValue = plot.x.min + (static_cast<double>(index) * plot.x.step);
        const int px = projectX(plotRect, plot, xValue);

        painter.drawLine(px, plotRect.top(), px, plotRect.bottom());

        painter.setPen(QPen(Qt::black, 1));
        painter.drawText(px - 15, plotRect.bottom() + 18, 30, 16, Qt::AlignCenter, QString::number(xValue));
        painter.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));
    }
}

void drawYGrid(QPainter &painter, const QRect &plotRect, const domain::PlotModel &plot) {
    const int count = tickCount(plot.y.min, plot.y.max, plot.y.step);
    if (count <= 0) {
        return;
    }

    painter.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));

    for (int index = 0; index < count; ++index) {
        const double yValue = plot.y.min + (static_cast<double>(index) * plot.y.step);
        const int py = projectY(plotRect, plot, yValue);

        painter.drawLine(plotRect.left(), py, plotRect.right(), py);

        painter.setPen(QPen(Qt::black, 1));
        painter.drawText(0, py - 8, leftMargin - 5, 16, Qt::AlignRight | Qt::AlignVCenter, QString::number(yValue));
        painter.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));
    }
}

QPolygon buildPolyline(const QRect &plotRect, const domain::PlotModel &plot) {
    QPolygon polyline;
    polyline.reserve(static_cast<int>(plot.series.points.size()));

    for (const auto &point : plot.series.points) {
        polyline << QPoint(projectX(plotRect, plot, point.x), projectY(plotRect, plot, point.y));
    }

    return polyline;
}

void drawSeries(QPainter &painter, const QRect &plotRect, const domain::PlotModel &plot) {
    const QPolygon polyline = buildPolyline(plotRect, plot);

    painter.setPen(QPen(toQColor(plot.color), 2));
    painter.drawPolyline(polyline);
}

} // namespace

PlotWidget::PlotWidget(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(220);
}

void PlotWidget::setPlot(domain::PlotModel plot) {
    this->plot = std::move(plot);
    update();
}

const domain::PlotModel &PlotWidget::getPlot() const {
    return plot;
}

bool PlotWidget::hasRenderablePlot(const domain::PlotModel &plotModel) {
    return plotModel.series.points.size() >= 2 && plotModel.x.max > plotModel.x.min &&
           plotModel.y.max > plotModel.y.min;
}

double PlotWidget::normalize(double value, double min, double max) {
    if (max <= min) {
        return 0.0;
    }

    return (value - min) / (max - min);
}

void PlotWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    drawBackground(painter, this);

    const QRect plotRect = makePlotRect(this);

    drawFrame(painter, plotRect);
    drawTitle(painter, this, plot);
    drawAxisLabels(painter, plotRect, plot);

    if (!hasRenderablePlot(plot)) {
        painter.drawText(plotRect, Qt::AlignCenter, QStringLiteral("No plot data"));
        return;
    }

    drawXGrid(painter, plotRect, plot);
    drawYGrid(painter, plotRect, plot);
    drawSeries(painter, plotRect, plot);
}

} // namespace ui
