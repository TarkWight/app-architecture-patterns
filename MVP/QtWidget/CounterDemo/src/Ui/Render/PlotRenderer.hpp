#ifndef PLOTRENDERER_HPP
#define PLOTRENDERER_HPP

#include <QPainter>
#include <QRect>

#include <vector>

#include "../../Application/Dto/PlotModel.hpp"

namespace ui::render {

class PlotRenderer final {
  public:
    static void drawPlot(QPainter &painter, const QRect &rect, const application::dto::PlotModel &plot);
    static std::vector<application::dto::Series> splitDrawableSegments(const application::dto::Series &series);

  private:
    static bool hasRenderablePlot(const application::dto::PlotModel &plot);
    static double normalize(double value, double min, double max);

    static QColor toQColor(application::dto::RgbColor color);

    static int projectX(const QRect &plotRect, const application::dto::PlotModel &plot, double xValue);

    static int projectY(const QRect &plotRect, const application::dto::PlotModel &plot, double yValue);

    static int tickCount(double minValue, double maxValue, double stepValue);

    static QString formatTickValue(double value, const application::dto::AxisSpec &axis);

    static void drawFrame(QPainter &painter, const QRect &plotRect);

    static void drawTitle(QPainter &painter, const QRect &outerRect, const application::dto::PlotModel &plot);

    static void drawAxisLabels(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot);

    static void drawXGrid(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot,
                          int leftMargin);

    static void drawYGrid(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot,
                          int leftMargin);

    static void drawMarker(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot);

    static QPolygon buildPolyline(const QRect &plotRect, const application::dto::PlotModel &plot,
                                  const application::dto::Series &series);

    static void drawPoint(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot,
                          const application::dto::Point &point);

    static void drawSeries(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot);

    static void drawLegend(QPainter &painter, const QRect &plotRect, const application::dto::PlotModel &plot);
};

} // namespace ui::render

#endif // PLOTRENDERER_HPP
