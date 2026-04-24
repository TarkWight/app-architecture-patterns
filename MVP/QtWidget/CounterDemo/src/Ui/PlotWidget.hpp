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

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    domain::PlotModel plot{};
};

} // namespace ui

#endif // PLOTWIDGET_HPP
