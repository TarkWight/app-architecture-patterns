#include "MainWindow.hpp"

#include "../Presentation/ViewModels/TestTimeViewModel.hpp"
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

    ui->comboBoxTestTimeSource->addItem(QStringLiteral("Авторасчёт"));
    ui->comboBoxTestTimeSource->addItem(QStringLiteral("Время оператора"));
    ui->comboBoxTestTimeSource->addItem(QStringLiteral("Свободный режим"));

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

void MainWindow::setPauseEnabled(bool enabled) {
    ui->buttonPause->setEnabled(enabled);
}

void MainWindow::setResumeEnabled(bool enabled) {
    ui->buttonResume->setEnabled(enabled);
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

    QObject::connect(ui->buttonPause, &QPushButton::clicked, this, [this]() { shellPresenter.onPausePressed(); });

    QObject::connect(ui->buttonResume, &QPushButton::clicked, this, [this]() { shellPresenter.onResumePressed(); });

    QObject::connect(ui->buttonStop, &QPushButton::clicked, this, [this]() { shellPresenter.onStopPressed(); });

    QObject::connect(ui->buttonCalculate, &QPushButton::clicked, this, [this]() { shellPresenter.onCalculatePressed(); });

    QObject::connect(ui->lineEditFormula, &QLineEdit::editingFinished, this, [this]() { shellPresenter.onFunctionEdited(ui->lineEditFormula->text().toStdString()); });

    QObject::connect(ui->buttonPickColor, &QPushButton::clicked, this, [this]() {
        const QColor color = QColorDialog::getColor(Qt::red, this);
        if (!color.isValid()) {
            return;
        }

        shellPresenter.onLineColorSelected(MainWindowUiAdapter::toDomainColor(color));
    });

    QObject::connect(ui->comboBoxTestTimeSource, &QComboBox::currentIndexChanged, this,
                     [this](int index) {
                         domain::TestTimeSource source = domain::TestTimeSource::AutoCalculated;

                         switch (index) {
                             case 0:
                                 source = domain::TestTimeSource::AutoCalculated;
                                 break;
                             case 1:
                                 source = domain::TestTimeSource::OperatorDefined;
                                 break;
                             case 2:
                                 source = domain::TestTimeSource::FreeRun;
                                 break;
                             default:
                                 return;
                         }

                         shellPresenter.onTestTimeSourceChanged(source);
                     });
}

void MainWindow::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::testTimeModelChanged, this,
                     [this](const presentation::viewModels::TestTimeViewModel & /*model*/) {
                         shellPresenter.onStateChanged();
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::functionExpressionChanged, this,
                     [this](const QString &expression) {
                         if (ui->lineEditFormula->text() == expression) {
                             return;
                         }

                         ui->lineEditFormula->setText(expression);
                     });
}

void MainWindow::setTestTimeSource(domain::TestTimeSource source) {
    int index = 0;

    switch (source) {
        case domain::TestTimeSource::AutoCalculated:
            index = 0;
            break;
        case domain::TestTimeSource::OperatorDefined:
            index = 1;
            break;
        case domain::TestTimeSource::FreeRun:
            index = 2;
            break;
    }

    if (ui->comboBoxTestTimeSource->currentIndex() != index) {
        ui->comboBoxTestTimeSource->setCurrentIndex(index);
    }
}

} // namespace ui
