#include "MainWindow.hpp"

#include "MainWindowUiAdapter.hpp"
#include "TelemetryChartsTabWidget.hpp"
#include "ControlChartsTabWidget.hpp"
#include "TestProtocolTabWidget.hpp"
#include "ui_MainWindow.h"

#include <QColorDialog>
#include <QString>

namespace ui {

MainWindow::MainWindow(Dependencies deps, QWidget *parent)
    : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()), shellPresenter(deps.shellPresenter),
      telemetryChartsTabPresenter(deps.telemetryChartsTabPresenter),
      controlChartsTabPresenter(deps.controlChartsTabPresenter),
      testProtocolTabPresenter(deps.testProtocolTabPresenter), sessionAdapter(deps.sessionAdapter) {
    ui->setupUi(this);

    shellPresenter.attachView(*this);

    setupTabs();
    connectShellSignals();
    connectSessionSignals();

    shellPresenter.onViewReady();
    telemetryChartsTabPresenter.onViewReady();
    controlChartsTabPresenter.onViewReady();
    testProtocolTabPresenter.onViewReady();
}

MainWindow::~MainWindow() {
    shellPresenter.detachView();
}

void MainWindow::setTimerText(const std::string &text) {
    ui->labelTimerValue->setText(QString::fromStdString(text));
}

void MainWindow::setStartEnabled(bool enabled) {
    ui->buttonStart->setEnabled(enabled);
}

void MainWindow::setStopEnabled(bool enabled) {
    ui->buttonStop->setEnabled(enabled);
}

void MainWindow::setFunctionExpression(const std::string &expression) {
    if (ui->lineEditFormula->text().toStdString() == expression) {
        return;
    }

    ui->lineEditFormula->setText(QString::fromStdString(expression));
}

void MainWindow::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void MainWindow::setupTabs() {
    telemetryChartsTabWidget = new TelemetryChartsTabWidget(telemetryChartsTabPresenter, sessionAdapter, this);
    controlChartsTabWidget = new ControlChartsTabWidget(controlChartsTabPresenter, sessionAdapter, this);
    testProtocolTabWidget = new TestProtocolTabWidget(testProtocolTabPresenter, sessionAdapter, this);

    ui->tabWidget->clear();
    ui->tabWidget->addTab(telemetryChartsTabWidget, QStringLiteral("Вкладка 1"));
    ui->tabWidget->addTab(controlChartsTabWidget, QStringLiteral("Вкладка 2"));
    ui->tabWidget->addTab(testProtocolTabWidget, QStringLiteral("Вкладка 3"));
}

void MainWindow::connectShellSignals() {
    QObject::connect(ui->buttonStart, &QPushButton::clicked, this, [this]() { shellPresenter.onStartPressed(); });

    QObject::connect(ui->buttonStop, &QPushButton::clicked, this, [this]() { shellPresenter.onStopPressed(); });

    QObject::connect(ui->buttonCalculate, &QPushButton::clicked, this,
                     [this]() { shellPresenter.onCalculatePressed(); });

    QObject::connect(ui->lineEditFormula, &QLineEdit::editingFinished, this,
                     [this]() { shellPresenter.onFunctionEdited(ui->lineEditFormula->text().toStdString()); });

    QObject::connect(ui->buttonPickColor, &QPushButton::clicked, this, [this]() {
        const QColor color = QColorDialog::getColor(Qt::red, this);
        if (!color.isValid()) {
            return;
        }

        shellPresenter.onLineColorSelected(MainWindowUiAdapter::toDomainColor(color));
    });
}

void MainWindow::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::timerChanged, this,
                     [this](int elapsedSeconds, bool /*running*/) {
                         setTimerText(MainWindowUiAdapter::formatElapsed(elapsedSeconds));
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::functionExpressionChanged, this,
                     [this](const QString &expression) {
                         if (ui->lineEditFormula->text() == expression) {
                             return;
                         }

                         ui->lineEditFormula->setText(expression);
                     });
}

} // namespace ui
