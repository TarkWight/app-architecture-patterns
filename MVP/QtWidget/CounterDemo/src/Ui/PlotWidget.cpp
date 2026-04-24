#include "PlotWidget.hpp"

#include "Render/PlotRenderer.hpp"

#include <QPaintEvent>
#include <QPainter>

namespace ui {

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

void PlotWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), palette().base());

    ui::render::PlotRenderer::drawPlot(painter, rect(), plot);
}

} // namespace ui
