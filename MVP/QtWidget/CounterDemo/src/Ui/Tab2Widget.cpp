#include "Tab2Widget.hpp"
#include "ui_Tab2Widget.h"

#include <QString>

namespace ui {

Tab2Widget::Tab2Widget(presentation::tab2::Tab2Presenter &presenter,
                       infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent)
    : QWidget(parent), ui(new Ui::Tab2Widget), presenter(presenter), sessionAdapter(sessionAdapter) {
    ui->setupUi(this);

    plotWidget = new PlotWidget(this);
    ui->verticalLayoutPlot->replaceWidget(ui->labelPlotState, plotWidget);
    ui->labelPlotState->hide();

    presenter.attachView(*this);

    connectSignals();
    connectSessionSignals();
}

Tab2Widget::~Tab2Widget() {
    presenter.detachView();
    delete ui;
}

void Tab2Widget::setMinutes(int minutes) {
    ui->spinBoxMinutes->setValue(minutes);
}

void Tab2Widget::refreshPlot() {
    plotWidget->setPlot(sessionAdapter.getState().get().plot2);
}

void Tab2Widget::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void Tab2Widget::connectSignals() {
    QObject::connect(ui->buttonRebuildPlot, &QPushButton::clicked, this,
                     [this]() { presenter.onRebuildPlotPressed(); });

    QObject::connect(ui->spinBoxMinutes, qOverload<int>(&QSpinBox::valueChanged), this,
                     [this](int value) { presenter.onMinutesChanged(value); });
}

void Tab2Widget::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::tab2MinutesChanged, this,
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
