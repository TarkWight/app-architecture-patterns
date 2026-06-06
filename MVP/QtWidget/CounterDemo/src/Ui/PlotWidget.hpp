#ifndef PLOTWIDGET_HPP
#define PLOTWIDGET_HPP

#include <QTimer>
#include <QWidget>

#include "../Domain/Plot.hpp"

namespace ui {

class PlotWidget final : public QWidget {
    Q_OBJECT

  public:
    explicit PlotWidget(QWidget *parent = nullptr);

    void setPlot(domain::PlotModel plot);
    const domain::PlotModel &getPlot() const;

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    domain::PlotModel plot{};
    domain::PlotModel targetPlot{};
    QTimer animationTimer{};

    void advanceAnimationFrame();

    static domain::PlotModel interpolatePlot(const domain::PlotModel &current, const domain::PlotModel &target);
    static domain::AxisSpec interpolateAxis(domain::AxisSpec current, domain::AxisSpec target);
    static domain::Series interpolateSeries(const domain::Series &current, const domain::Series &target);
    static domain::RgbColor interpolateColor(domain::RgbColor current, domain::RgbColor target);
    static double interpolateValue(double current, double target);
    static bool isClose(const domain::PlotModel &current, const domain::PlotModel &target);
};

} // namespace ui

#endif // PLOTWIDGET_HPP
