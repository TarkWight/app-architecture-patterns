#ifndef PLOTWIDGET_HPP
#define PLOTWIDGET_HPP

#include <QWidget>

#include "../Domain/Plot.hpp"

namespace ui {

class PlotWidget final : public QWidget {
    Q_OBJECT

  public:
    explicit PlotWidget(QWidget *parent = nullptr);

    void setPlot(domain::PlotModel plot);
    const domain::PlotModel &getPlot() const;

    static double normalize(double value, double min, double max);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    domain::PlotModel plot{};

    static bool hasRenderablePlot(const domain::PlotModel &plotModel);
};

} // namespace ui

#endif // PLOTWIDGET_HPP
