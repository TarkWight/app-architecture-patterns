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

void ControlChartsTabWidget::setBeaufort(double value) {
    ui->doubleSpinBoxBeaufort->setValue(value);
}

void ControlChartsTabWidget::setDirection(double value) {
    ui->doubleSpinBoxDirection->setValue(value);
}

void ControlChartsTabWidget::setAngleOfAttack(double value) {
    ui->doubleSpinBoxAngleOfAttack->setValue(value);
}

void ControlChartsTabWidget::refreshPlot() {
    plotWidget->setPlot(sessionAdapter.getState().get().controlPlot);
}

void ControlChartsTabWidget::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void ControlChartsTabWidget::connectSignals() {
    QObject::connect(ui->buttonRebuildPlot, &QPushButton::clicked, this,
                     [this]() { presenter.onRebuildPlotPressed(); });

    QObject::connect(ui->spinBoxMinutes, qOverload<int>(&QSpinBox::valueChanged), this,
                     [this](int value) { presenter.onMinutesChanged(value); });

    QObject::connect(ui->doubleSpinBoxBeaufort, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                     [this](double value) { presenter.onBeaufortChanged(value); });

    QObject::connect(ui->doubleSpinBoxDirection, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                     [this](double value) { presenter.onDirectionChanged(value); });

    QObject::connect(ui->doubleSpinBoxAngleOfAttack, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                     [this](double value) { presenter.onAngleOfAttackChanged(value); });
}

void ControlChartsTabWidget::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::controlChartsTabMinutesChanged, this,
                     [this](int minutes) {
                         if (ui->spinBoxMinutes->value() == minutes) {
                             return;
                         }

                         ui->spinBoxMinutes->setValue(minutes);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::beaufortChanged, this,
                     [this](double value) {
                         if (ui->doubleSpinBoxBeaufort->value() == value) {
                             return;
                         }

                         ui->doubleSpinBoxBeaufort->setValue(value);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::directionChanged, this,
                     [this](double value) {
                         if (ui->doubleSpinBoxDirection->value() == value) {
                             return;
                         }

                         ui->doubleSpinBoxDirection->setValue(value);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::angleOfAttackChanged, this,
                     [this](double value) {
                         if (ui->doubleSpinBoxAngleOfAttack->value() == value) {
                             return;
                         }

                         ui->doubleSpinBoxAngleOfAttack->setValue(value);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::controlPlotChanged, this,
                     [this]() { refreshPlot(); });
}

} // namespace ui
