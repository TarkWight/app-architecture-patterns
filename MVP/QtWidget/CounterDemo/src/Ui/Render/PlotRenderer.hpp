#ifndef PLOTRENDERER_HPP
#define PLOTRENDERER_HPP

#include <QPainter>
#include <QRect>

#include "../../Domain/Plot.hpp"

namespace ui::render {

class PlotRenderer final {
  public:
    static void drawPlot(QPainter &painter, const QRect &rect, const domain::PlotModel &plot);

  private:
    static bool hasRenderablePlot(const domain::PlotModel &plot);
    static double normalize(double value, double min, double max);

    static QColor toQColor(domain::RgbColor color);

    static int projectX(const QRect &plotRect, const domain::PlotModel &plot, double xValue);

    static int projectY(const QRect &plotRect, const domain::PlotModel &plot, double yValue);

    static int tickCount(double minValue, double maxValue, double stepValue);

    static void drawFrame(QPainter &painter, const QRect &plotRect);

    static void drawTitle(QPainter &painter, const QRect &outerRect, const domain::PlotModel &plot);

    static void drawAxisLabels(QPainter &painter, const QRect &plotRect, const domain::PlotModel &plot);

    static void drawXGrid(QPainter &painter, const QRect &plotRect, const domain::PlotModel &plot, int leftMargin);

    static void drawYGrid(QPainter &painter, const QRect &plotRect, const domain::PlotModel &plot, int leftMargin);

    static QPolygon buildPolyline(const QRect &plotRect, const domain::PlotModel &plot);

    static void drawSeries(QPainter &painter, const QRect &plotRect, const domain::PlotModel &plot);
};

} // namespace ui::render

#endif // PLOTRENDERER_HPP
