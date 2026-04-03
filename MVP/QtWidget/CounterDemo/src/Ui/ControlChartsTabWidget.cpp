#include "ControlChartsTabWidget.hpp"
#include "ui_ControlChartsTabWidget.h"

#include <QString>

namespace ui {

ControlChartsTabWidget::ControlChartsTabWidget(presentation::controlChartsTab::ControlChartsTabPresenter &presenter,
                                               infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent)
    : QWidget(parent), ui(new Ui::ControlChartsTabWidget), presenter(presenter), sessionAdapter(sessionAdapter) {
    ui->setupUi(this);

    plotWidget = new PlotWidget(this);
    ui->verticalLayoutPlot->replaceWidget(ui->labelPlotState, plotWidget);
    ui->labelPlotState->hide();

    presenter.attachView(*this);

    connectSignals();
    connectSessionSignals();
}

ControlChartsTabWidget::~ControlChartsTabWidget() {
    presenter.detachView();
    delete ui;
}

void ControlChartsTabWidget::setMinutes(int minutes) {
    ui->spinBoxMinutes->setValue(minutes);
}

void ControlChartsTabWidget::refreshPlot() {
    plotWidget->setPlot(sessionAdapter.getState().get().plot2);
}

void ControlChartsTabWidget::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void ControlChartsTabWidget::connectSignals() {
    QObject::connect(ui->buttonRebuildPlot, &QPushButton::clicked, this,
                     [this]() { presenter.onRebuildPlotPressed(); });

    QObject::connect(ui->spinBoxMinutes, qOverload<int>(&QSpinBox::valueChanged), this,
                     [this](int value) { presenter.onMinutesChanged(value); });
}

void ControlChartsTabWidget::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::controlChartsTabMinutesChanged, this,
                     [this](int minutes) {
                         if (ui->spinBoxMinutes->value() == minutes) {
                             return;
                         }

                         ui->spinBoxMinutes->setValue(minutes);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::plot2Changed, this,
                     [this]() { refreshPlot(); });
}

} // namespace ui
