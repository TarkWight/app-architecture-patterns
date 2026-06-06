#include "MainWindow.hpp"

#include "../Presentation/ViewModels/TestTimeViewModel.hpp"
#include "MainWindowUiAdapter.hpp"
#include "TelemetryChartsTabWidget.hpp"
#include "ControlChartsTabWidget.hpp"
#include "TestProtocolTabWidget.hpp"
#include "ui_MainWindow.h"

#include "../Domain/AxisId.hpp"

#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QString>
#include <QVBoxLayout>

#include <array>
#include <cmath>
#include <utility>

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
    setupMainContentLayout();
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

void MainWindow::setStandConnectionButtonText(const std::string &text) {
    ui->buttonConnectTelemetry->setText(QString::fromStdString(text));
}

void MainWindow::setFunctionExpression(const std::string &expression) {
    if (controlFormulaLineEdit == nullptr || controlFormulaLineEdit->text().toStdString() == expression) {
        return;
    }

    controlFormulaLineEdit->setText(QString::fromStdString(expression));
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

void MainWindow::setupMainContentLayout() {
    ui->labelFormulaCaption->hide();
    ui->lineEditFormula->hide();
    ui->buttonCalculate->hide();
    ui->buttonPickColor->hide();

    const int tabIndex = ui->verticalLayoutRoot->indexOf(ui->tabWidget);
    ui->verticalLayoutRoot->removeWidget(ui->tabWidget);

    auto *contentWidget = new QWidget(this);
    auto *contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(8);
    contentLayout->addWidget(createStandControlPanel());
    contentLayout->addWidget(ui->tabWidget, 1);

    ui->verticalLayoutRoot->insertWidget(tabIndex, contentWidget, 1);

    controlChartsTabWidget->insertTopPanel(*createControlFormulaPanel());
}

QWidget *MainWindow::createStandControlPanel() {
    auto *panel = new QFrame(this);
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setMinimumWidth(250);
    panel->setMaximumWidth(290);

    auto *layout = new QVBoxLayout(panel);
    layout->setSpacing(8);

    auto *title = new QLabel(QStringLiteral("Управление стендом"), panel);
    auto titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignLeft);

    standBeaufortSpinBox = new QDoubleSpinBox(panel);
    standBeaufortSpinBox->setDecimals(1);
    standBeaufortSpinBox->setRange(0.0, 12.0);
    standBeaufortSpinBox->setSingleStep(0.1);
    form->addRow(QStringLiteral("Бофорт"), standBeaufortSpinBox);

    standAngleOfAttackSpinBox = new QDoubleSpinBox(panel);
    standAngleOfAttackSpinBox->setDecimals(1);
    standAngleOfAttackSpinBox->setRange(-90.0, 90.0);
    standAngleOfAttackSpinBox->setSingleStep(1.0);
    standAngleOfAttackSpinBox->setSuffix(QStringLiteral(" °"));
    form->addRow(QStringLiteral("Угол атаки"), standAngleOfAttackSpinBox);

    standDirectionComboBox = new QComboBox(panel);
    const std::array<std::pair<const char *, double>, 16> directions{{
        {"N", 0.0},
        {"NNE", 22.5},
        {"NE", 45.0},
        {"ENE", 67.5},
        {"E", 90.0},
        {"ESE", 112.5},
        {"SE", 135.0},
        {"SSE", 157.5},
        {"S", 180.0},
        {"SSW", 202.5},
        {"SW", 225.0},
        {"WSW", 247.5},
        {"W", 270.0},
        {"WNW", 292.5},
        {"NW", 315.0},
        {"NNW", 337.5},
    }};

    for (const auto &[label, degrees] : directions) {
        standDirectionComboBox->addItem(
            QStringLiteral("%1 (%2°)").arg(QString::fromUtf8(label)).arg(degrees, 0, 'f', 1), degrees);
    }
    form->addRow(QStringLiteral("Сторона света"), standDirectionComboBox);

    telemetryAxisComboBox = new QComboBox(panel);
    telemetryAxisComboBox->addItem(QStringLiteral("Ось Y / тангаж"), 0);
    telemetryAxisComboBox->addItem(QStringLiteral("Ось Z / направление"), 1);
    form->addRow(QStringLiteral("Линия графика"), telemetryAxisComboBox);

    layout->addLayout(form);

    auto *applyButton = new QPushButton(QStringLiteral("Применить воздействие"), panel);
    QObject::connect(applyButton, &QPushButton::clicked, this, [this]() { applyStandInputs(); });
    layout->addWidget(applyButton);

    auto *colorButton = new QPushButton(QStringLiteral("Цвет линии"), panel);
    QObject::connect(colorButton, &QPushButton::clicked, this, [this]() { selectTelemetryAxisColor(); });
    layout->addWidget(colorButton);

    layout->addStretch(1);

    return panel;
}

QWidget *MainWindow::createControlFormulaPanel() {
    auto *panel = new QFrame(controlChartsTabWidget);
    panel->setFrameShape(QFrame::StyledPanel);

    auto *layout = new QHBoxLayout(panel);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(8);

    layout->addWidget(new QLabel(QStringLiteral("Формула:"), panel));

    controlFormulaLineEdit = new QLineEdit(panel);
    controlFormulaLineEdit->setPlaceholderText(QStringLiteral("Введите формулу управляющего воздействия"));
    controlFormulaLineEdit->setText(ui->lineEditFormula->text());
    layout->addWidget(controlFormulaLineEdit, 1);

    auto *calculateButton = new QPushButton(QStringLiteral("Рассчитать"), panel);
    QObject::connect(calculateButton, &QPushButton::clicked, this, [this]() { shellPresenter.onCalculatePressed(); });
    layout->addWidget(calculateButton);

    auto *colorButton = new QPushButton(QStringLiteral("Цвет графика"), panel);
    QObject::connect(colorButton, &QPushButton::clicked, this, [this]() {
        const QColor color = QColorDialog::getColor(Qt::red, this);
        if (!color.isValid()) {
            return;
        }

        shellPresenter.onLineColorSelected(MainWindowUiAdapter::toDomainColor(color));
    });
    layout->addWidget(colorButton);

    return panel;
}

void MainWindow::connectShellSignals() {
    QObject::connect(ui->buttonStart, &QPushButton::clicked, this, [this]() { shellPresenter.onStartPressed(); });

    QObject::connect(ui->buttonPause, &QPushButton::clicked, this, [this]() { shellPresenter.onPausePressed(); });

    QObject::connect(ui->buttonResume, &QPushButton::clicked, this, [this]() { shellPresenter.onResumePressed(); });

    QObject::connect(ui->buttonStop, &QPushButton::clicked, this, [this]() { shellPresenter.onStopPressed(); });

    QObject::connect(controlFormulaLineEdit, &QLineEdit::editingFinished, this,
                     [this]() { shellPresenter.onFunctionEdited(controlFormulaLineEdit->text().toStdString()); });

    QObject::connect(ui->comboBoxTestTimeSource, &QComboBox::currentIndexChanged, this, [this](int index) {
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

    QObject::connect(ui->buttonConnectTelemetry, &QPushButton::clicked, this, [this]() {
        shellPresenter.onConnectTelemetryPressed(
            "/Users/tarkwight/Documents/Development/app-architecture-patterns/MVP/QtWidget/CounterDemo/telemetry.toml");
    });
}

void MainWindow::connectSessionSignals() {
    QObject::connect(
        &sessionAdapter, &infrastructure::SessionStateQtAdapter::testTimeModelChanged, this,
        [this](const presentation::viewModels::TestTimeViewModel & /*model*/) { shellPresenter.onStateChanged(); });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::functionExpressionChanged, this,
                     [this](const QString &expression) {
                         if (controlFormulaLineEdit == nullptr || controlFormulaLineEdit->text() == expression) {
                             return;
                         }

                         controlFormulaLineEdit->setText(expression);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::beaufortChanged, this,
                     [this](double value) {
                         if (standBeaufortSpinBox == nullptr || standBeaufortSpinBox->value() == value) {
                             return;
                         }

                         const QSignalBlocker blocker{standBeaufortSpinBox};
                         standBeaufortSpinBox->setValue(value);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::angleOfAttackChanged, this,
                     [this](double value) {
                         if (standAngleOfAttackSpinBox == nullptr || standAngleOfAttackSpinBox->value() == value) {
                             return;
                         }

                         const QSignalBlocker blocker{standAngleOfAttackSpinBox};
                         standAngleOfAttackSpinBox->setValue(value);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::directionChanged, this,
                     [this](double value) {
                         if (standDirectionComboBox == nullptr) {
                             return;
                         }

                         int bestIndex = 0;
                         double bestDistance = 360.0;
                         for (int index = 0; index < standDirectionComboBox->count(); ++index) {
                             const double direction = standDirectionComboBox->itemData(index).toDouble();
                             const double distance = std::abs(direction - value);
                             if (distance < bestDistance) {
                                 bestDistance = distance;
                                 bestIndex = index;
                             }
                         }

                         const QSignalBlocker blocker{standDirectionComboBox};
                         standDirectionComboBox->setCurrentIndex(bestIndex);
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

void MainWindow::applyStandInputs() {
    controlChartsTabPresenter.onBeaufortChanged(standBeaufortSpinBox->value());
    controlChartsTabPresenter.onAngleOfAttackChanged(standAngleOfAttackSpinBox->value());
    controlChartsTabPresenter.onDirectionChanged(selectedStandDirectionDegrees());
    controlChartsTabPresenter.onRebuildPlotPressed();

    appendLog("Stand impact parameters applied");
}

void MainWindow::selectTelemetryAxisColor() {
    const QColor color = QColorDialog::getColor(Qt::red, this);
    if (!color.isValid()) {
        return;
    }

    const int axisIndex = telemetryAxisComboBox->currentData().toInt();
    const domain::AxisId axisId = axisIndex == 0 ? domain::axis0 : domain::axis1;

    telemetryChartsTabPresenter.onTelemetryAxisColorSelected(axisId, MainWindowUiAdapter::toDomainColor(color));
}

double MainWindow::selectedStandDirectionDegrees() const {
    return standDirectionComboBox->currentData().toDouble();
}

} // namespace ui
