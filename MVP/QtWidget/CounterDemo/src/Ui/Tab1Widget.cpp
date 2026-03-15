#include "Tab1Widget.hpp"
#include "ui_Tab1Widget.h"

#include <QString>

namespace ui {

Tab1Widget::Tab1Widget(presentation::tab1::Tab1Presenter &presenter,
                       infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent)
    : QWidget(parent), ui(new Ui::Tab1Widget), presenter(presenter), sessionAdapter(sessionAdapter) {
    ui->setupUi(this);

    plotWidget = new PlotWidget(this);
    ui->verticalLayoutPlot->replaceWidget(ui->labelPlotState, plotWidget);
    ui->labelPlotState->hide();

    presenter.attachView(*this);

    connectSignals();
    connectSessionSignals();
}

Tab1Widget::~Tab1Widget() {
    presenter.detachView();
    delete ui;
}

void Tab1Widget::setCounterValue(int value) {
    ui->labelCounterValue->setText(QString::number(value));
}

void Tab1Widget::refreshPlot() {
    plotWidget->setPlot(sessionAdapter.getState().get().plot1);
}

void Tab1Widget::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void Tab1Widget::connectSignals() {
    QObject::connect(ui->buttonIncrement, &QPushButton::clicked, this, [this]() { presenter.onIncrementPressed(); });

    QObject::connect(ui->buttonDecrement, &QPushButton::clicked, this, [this]() { presenter.onDecrementPressed(); });

    QObject::connect(ui->buttonReset, &QPushButton::clicked, this, [this]() { presenter.onResetPressed(); });

    QObject::connect(ui->buttonRebuildPlot, &QPushButton::clicked, this,
                     [this]() { presenter.onRebuildPlotPressed(); });
}

void Tab1Widget::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::plot1Changed, this,
                     [this]() { refreshPlot(); });
}

} // namespace ui
