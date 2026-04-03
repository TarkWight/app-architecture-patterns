#include "TelemetryChartsTabWidget.hpp"
#include "ui_TelemetryChartsTabWidget.h"

#include <QString>

namespace ui {

TelemetryChartsTabWidget::TelemetryChartsTabWidget(
    presentation::telemetryChartsTab::TelemetryChartsTabPresenter &presenter,
    infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent)
    : QWidget(parent), ui(new Ui::TelemetryChartsTabWidget), presenter(presenter), sessionAdapter(sessionAdapter) {
    ui->setupUi(this);

    plotWidget = new PlotWidget(this);
    ui->verticalLayoutPlot->replaceWidget(ui->labelPlotState, plotWidget);
    ui->labelPlotState->hide();

    presenter.attachView(*this);

    connectSignals();
    connectSessionSignals();
}

TelemetryChartsTabWidget::~TelemetryChartsTabWidget() {
    presenter.detachView();
    delete ui;
}

void TelemetryChartsTabWidget::refreshPlot() {
    plotWidget->setPlot(sessionAdapter.getState().get().plot1);
}

void TelemetryChartsTabWidget::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void TelemetryChartsTabWidget::connectSignals() {
    QObject::connect(ui->buttonRebuildPlot, &QPushButton::clicked, this,
                     [this]() { presenter.onRebuildPlotPressed(); });
}

void TelemetryChartsTabWidget::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::plot1Changed, this,
                     [this]() { refreshPlot(); });
}

} // namespace ui
