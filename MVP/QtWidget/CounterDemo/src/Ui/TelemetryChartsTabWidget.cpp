#include "TelemetryChartsTabWidget.hpp"
#include "ui_TelemetryChartsTabWidget.h"

#include "../Localization/UiStrings.hpp"

#include <QScrollBar>
#include <QSignalBlocker>
#include <QString>

#include <algorithm>
#include <cmath>

namespace ui {

namespace {

QString uiText(const char *text) {
    return QString::fromUtf8(text);
}

} // namespace

TelemetryChartsTabWidget::TelemetryChartsTabWidget(
    presentation::telemetryChartsTab::TelemetryChartsTabPresenter &presenter,
    infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent)
    : QWidget(parent), ui(new Ui::TelemetryChartsTabWidget), presenter(presenter), sessionAdapter(sessionAdapter) {
    ui->setupUi(this);

    plotWidget = new PlotWidget(this);
    ui->verticalLayoutPlot->replaceWidget(ui->labelPlotState, plotWidget);
    ui->labelPlotState->hide();

    telemetryScrollBar = new QScrollBar(Qt::Horizontal, this);
    ui->verticalLayoutPlot->addWidget(telemetryScrollBar);
    ui->buttonRebuildPlot->setText(uiText(localization::ui::telemetryTailButton));

    presenter.attachView(*this);

    connectSignals();
    connectSessionSignals();
}

TelemetryChartsTabWidget::~TelemetryChartsTabWidget() {
    presenter.detachView();
    delete ui;
}

void TelemetryChartsTabWidget::refreshPlot() {
    plotWidget->setPlot(sessionAdapter.getState().get().telemetry.telemetryPlot);
    refreshTelemetryScrollBar();
}

void TelemetryChartsTabWidget::appendLog(const std::string &text) {
    emit logMessage(QString::fromStdString(text));
}

void TelemetryChartsTabWidget::connectSignals() {
    QObject::connect(ui->buttonRebuildPlot, &QPushButton::clicked, this, [this]() {
        telemetryAutoFollowTail = true;
        presenter.onRebuildPlotPressed();
    });

    QObject::connect(telemetryScrollBar, &QScrollBar::valueChanged, this, [this](int value) {
        if (value >= telemetryScrollBar->maximum()) {
            telemetryAutoFollowTail = true;
            presenter.onRebuildPlotPressed();
            return;
        }

        telemetryAutoFollowTail = false;
        presenter.onTelemetryWindowChanged(value);
    });
}

void TelemetryChartsTabWidget::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::telemetryPlotChanged, this,
                     [this]() { refreshPlot(); });
}

void TelemetryChartsTabWidget::refreshTelemetryScrollBar() {
    const auto &stateData = sessionAdapter.getState().get();
    const int historyEndSeconds = telemetryHistoryEndSeconds();
    const int selectedEndSeconds = static_cast<int>(std::ceil(stateData.telemetry.telemetryWindowEndSeconds.seconds()));
    const int windowSeconds = static_cast<int>(std::ceil(stateData.telemetry.telemetryWindowSeconds));

    const QSignalBlocker blocker{telemetryScrollBar};

    telemetryScrollBar->setRange(0, std::max(0, historyEndSeconds));
    telemetryScrollBar->setPageStep(std::max(1, windowSeconds));
    telemetryScrollBar->setSingleStep(1);

    if (telemetryAutoFollowTail) {
        telemetryScrollBar->setValue(telemetryScrollBar->maximum());
        return;
    }

    telemetryScrollBar->setValue(
        std::clamp(selectedEndSeconds, telemetryScrollBar->minimum(), telemetryScrollBar->maximum()));
}

int TelemetryChartsTabWidget::telemetryHistoryEndSeconds() const {
    const auto &stateData = sessionAdapter.getState().get();
    if (stateData.telemetry.telemetryHistory.empty()) {
        return 0;
    }

    const double baseTimestamp = stateData.telemetry.telemetryHistory.front().timestampSeconds;
    return static_cast<int>(
        std::ceil(std::max(0.0, stateData.telemetry.telemetryHistory.back().timestampSeconds - baseTimestamp)));
}

} // namespace ui
