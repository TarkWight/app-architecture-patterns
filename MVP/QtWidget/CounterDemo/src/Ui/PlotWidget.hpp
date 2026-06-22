#ifndef PLOTWIDGET_HPP
#define PLOTWIDGET_HPP

#include <QTimer>
#include <QWidget>

#include "../Application/Dto/PlotModel.hpp"

namespace ui {

class PlotWidget final : public QWidget {
    Q_OBJECT

  public:
    explicit PlotWidget(QWidget *parent = nullptr);

    void setPlot(application::dto::PlotModel plot);
    const application::dto::PlotModel &getPlot() const;

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    application::dto::PlotModel plot{};
    application::dto::PlotModel targetPlot{};
    QTimer animationTimer{};

    void advanceAnimationFrame();

    static application::dto::PlotModel interpolatePlot(const application::dto::PlotModel &current,
                                                       const application::dto::PlotModel &target);
    static application::dto::AxisSpec interpolateAxis(application::dto::AxisSpec current,
                                                      application::dto::AxisSpec target);
    static application::dto::Series interpolateSeries(const application::dto::Series &current,
                                                      const application::dto::Series &target);
    static application::dto::RgbColor interpolateColor(application::dto::RgbColor current,
                                                       application::dto::RgbColor target);
    static double interpolateValue(double current, double target);
    static bool isClose(const application::dto::PlotModel &current, const application::dto::PlotModel &target);
    static bool isClose(const application::dto::AxisSpec &current, const application::dto::AxisSpec &target);
    static bool isClose(const application::dto::Series &current, const application::dto::Series &target);
    static bool isClose(const application::dto::NamedSeries &current, const application::dto::NamedSeries &target);
};

} // namespace ui

#endif // PLOTWIDGET_HPP
