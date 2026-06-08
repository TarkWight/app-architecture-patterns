#include "MainWindow.hpp"

#include "../Presentation/ViewModels/TestTimeViewModel.hpp"
#include "MainWindowUiAdapter.hpp"
#include "TelemetryChartsTabWidget.hpp"
#include "ControlChartsTabWidget.hpp"
#include "TestProtocolTabWidget.hpp"
#include "ui_MainWindow.h"

#include "../Domain/AxisId.hpp"
#include "../Domain/FormulaTemplate.hpp"
#include "../Domain/StandControlMode.hpp"
#include "../Domain/StandImpactTransition.hpp"
#include "../Domain/StandScenario.hpp"
#include "../Domain/WindImpact.hpp"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>

#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

namespace ui {

namespace {

std::string compassLabel(double degrees) {
    constexpr std::array<const char *, 16> labels{"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
                                                  "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};

    const auto normalized = std::fmod(degrees + 360.0, 360.0);
    const auto index = static_cast<std::size_t>(std::lround(normalized / 22.5)) % labels.size();
    return labels[index];
}

std::string formatImpact(const domain::WindImpact &profile) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1) << "Bft=" << profile.beaufort.value()
        << ", dir=" << compassLabel(profile.direction.degrees()) << " (" << profile.direction.degrees()
        << " deg), AoA=" << profile.angleOfAttack.degrees() << " deg";
    return out.str();
}

} // namespace

MainWindow::MainWindow(Dependencies deps, QWidget *parent)
    : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()), shellPresenter(deps.shellPresenter),
      telemetryChartsTabPresenter(deps.telemetryChartsTabPresenter),
      controlChartsTabPresenter(deps.controlChartsTabPresenter),
      testProtocolTabPresenter(deps.testProtocolTabPresenter),
      setStandControlModeUseCase(deps.setStandControlModeUseCase), setStandImpactUseCase(deps.setStandImpactUseCase),
      sessionAdapter(deps.sessionAdapter) {
    ui->setupUi(this);

    ui->comboBoxTestTimeSource->addItem(QStringLiteral("Авторасчёт"));
    ui->comboBoxTestTimeSource->addItem(QStringLiteral("Время оператора"));
    ui->comboBoxTestTimeSource->addItem(QStringLiteral("Свободный режим"));

    standImpactTransitionTimer = new QTimer(this);
    standImpactTransitionTimer->setInterval(100);
    QObject::connect(standImpactTransitionTimer, &QTimer::timeout, this, &MainWindow::advanceStandImpactTransition);

    shellPresenter.attachView(*this);

    setupStandConnectionStatusIndicator();
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

void MainWindow::setStandConnectionStatusText(const std::string &text) {
    if (standConnectionStatusLabel == nullptr) {
        return;
    }

    standConnectionStatusLabel->setText(QString::fromStdString(text));
}

void MainWindow::setFunctionExpression(const std::string &expression) {
    updateControlFormulaTemplateSelection(expression);

    if (controlFormulaLineEdit == nullptr || controlFormulaLineEdit->hasFocus() ||
        controlFormulaLineEdit->text().toStdString() == expression) {
        return;
    }

    controlFormulaLineEdit->setText(QString::fromStdString(expression));
}

void MainWindow::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void MainWindow::showOperatorWarning(const std::string &title, const std::string &message) {
    QMessageBox::warning(this, QString::fromStdString(title), QString::fromStdString(message));
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

void MainWindow::setupStandConnectionStatusIndicator() {
    standConnectionStatusLabel = new QLabel(QStringLiteral("Стенд: отключен"), this);
    standConnectionStatusLabel->setMinimumWidth(170);
    standConnectionStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->gridLayoutTopPanel->addWidget(standConnectionStatusLabel, 1, 7, 1, 4);
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
    layout->addWidget(createPanelTitle(*panel, QStringLiteral("Управление стендом")));

    auto *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignLeft);
    addStandImpactRows(*form, *panel);
    addTelemetryBindingRows(*form, *panel);

    layout->addLayout(form);
    addStandControlButtons(*layout, *panel);
    layout->addStretch(1);

    return panel;
}

QLabel *MainWindow::createPanelTitle(QWidget &parent, const QString &text) const {
    auto *title = new QLabel(text, &parent);
    auto titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    return title;
}

void MainWindow::addStandImpactRows(QFormLayout &form, QWidget &parent) {
    standControlModeComboBox = new QComboBox(&parent);
    standControlModeComboBox->addItem(QStringLiteral("Ручной"), static_cast<int>(domain::StandControlMode::Manual));
    standControlModeComboBox->addItem(QStringLiteral("Гибридный"), static_cast<int>(domain::StandControlMode::Hybrid));
    standControlModeComboBox->addItem(QStringLiteral("Заготовленный"),
                                      static_cast<int>(domain::StandControlMode::PresetScenario));
    form.addRow(QStringLiteral("Вид теста"), standControlModeComboBox);

    standBeaufortSpinBox = new QDoubleSpinBox(&parent);
    standBeaufortSpinBox->setDecimals(1);
    standBeaufortSpinBox->setRange(domain::minOperationalBeaufort, domain::maxOperationalBeaufort);
    standBeaufortSpinBox->setSingleStep(0.1);
    form.addRow(QStringLiteral("Бофорт"), standBeaufortSpinBox);

    standAngleOfAttackSpinBox = new QDoubleSpinBox(&parent);
    standAngleOfAttackSpinBox->setDecimals(1);
    standAngleOfAttackSpinBox->setRange(domain::minAngleOfAttack, domain::maxAngleOfAttack);
    standAngleOfAttackSpinBox->setSingleStep(1.0);
    standAngleOfAttackSpinBox->setSuffix(QStringLiteral(" °"));
    form.addRow(QStringLiteral("Угол атаки"), standAngleOfAttackSpinBox);

    standDirectionComboBox = new QComboBox(&parent);
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
    form.addRow(QStringLiteral("Сторона света"), standDirectionComboBox);
}

void MainWindow::addTelemetryBindingRows(QFormLayout &form, QWidget &parent) {
    telemetryCurveComboBox = new QComboBox(&parent);
    telemetryCurveComboBox->addItem(QStringLiteral("Кривая 1"), 0);
    telemetryCurveComboBox->addItem(QStringLiteral("Кривая 2"), 1);
    form.addRow(QStringLiteral("Кривая"), telemetryCurveComboBox);

    telemetrySourceComboBox = new QComboBox(&parent);
    telemetrySourceComboBox->addItem(QStringLiteral("Ось Y / тангаж"), 0);
    telemetrySourceComboBox->addItem(QStringLiteral("Ось Z / направление"), 1);
    form.addRow(QStringLiteral("Источник"), telemetrySourceComboBox);

    telemetryCurveVisibleCheckBox = new QCheckBox(QStringLiteral("Показывать"), &parent);
    telemetryCurveVisibleCheckBox->setChecked(true);
    form.addRow(QStringLiteral("Отображение"), telemetryCurveVisibleCheckBox);
}

void MainWindow::addStandControlButtons(QVBoxLayout &layout, QWidget &parent) {
    standApplyButton = new QPushButton(QStringLiteral("Применить воздействие"), &parent);
    QObject::connect(standApplyButton, &QPushButton::clicked, this, [this]() { applyStandInputs(); });
    layout.addWidget(standApplyButton);

    auto *colorButton = new QPushButton(QStringLiteral("Цвет линии"), &parent);
    QObject::connect(colorButton, &QPushButton::clicked, this, [this]() { selectTelemetryAxisColor(); });
    layout.addWidget(colorButton);
}

QWidget *MainWindow::createControlFormulaPanel() {
    auto *panel = new QFrame(controlChartsTabWidget);
    panel->setFrameShape(QFrame::StyledPanel);

    auto *layout = new QHBoxLayout(panel);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(8);

    layout->addWidget(new QLabel(QStringLiteral("Формула:"), panel));

    controlFormulaTemplateComboBox = new QComboBox(panel);
    controlFormulaTemplateComboBox->addItem(QStringLiteral("Своя формула"), QString{});
    for (const auto &formulaTemplate : domain::formulaTemplates) {
        controlFormulaTemplateComboBox->addItem(
            QString::fromUtf8(formulaTemplate.title.data(), static_cast<qsizetype>(formulaTemplate.title.size())),
            QString::fromUtf8(formulaTemplate.key.data(), static_cast<qsizetype>(formulaTemplate.key.size())));
    }
    layout->addWidget(controlFormulaTemplateComboBox);

    controlFormulaLineEdit = new QLineEdit(panel);
    controlFormulaLineEdit->setPlaceholderText(QStringLiteral("Введите формулу управляющего воздействия"));
    controlFormulaLineEdit->setText(ui->lineEditFormula->text());
    updateControlFormulaTemplateSelection(controlFormulaLineEdit->text().toStdString());
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

    QObject::connect(controlFormulaTemplateComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index <= 0) {
            return;
        }

        shellPresenter.onFormulaTemplateSelected(
            controlFormulaTemplateComboBox->currentData().toString().toStdString());
    });

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

    QObject::connect(standControlModeComboBox, &QComboBox::currentIndexChanged, this, [this]() {
        const auto mode = static_cast<domain::StandControlMode>(standControlModeComboBox->currentData().toInt());
        setStandControlModeUseCase.execute(mode);
        updateManualStandControlsEnabled();
    });

    QObject::connect(telemetryCurveComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        const QSignalBlocker sourceBlocker{telemetrySourceComboBox};
        const QSignalBlocker visibleBlocker{telemetryCurveVisibleCheckBox};

        telemetrySourceComboBox->setCurrentIndex(index);

        const auto &stateData = sessionAdapter.getState().get();
        telemetryCurveVisibleCheckBox->setChecked(index == 0 ? stateData.telemetryAxisYVisible
                                                             : stateData.telemetryAxisZVisible);
    });

    QObject::connect(telemetrySourceComboBox, &QComboBox::currentIndexChanged, this,
                     [this](int index) { telemetryCurveComboBox->setCurrentIndex(index); });

    QObject::connect(telemetryCurveVisibleCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        telemetryChartsTabPresenter.onTelemetryAxisVisibilityChanged(selectedTelemetryAxisId(), checked);
    });
}

void MainWindow::connectSessionSignals() {
    QObject::connect(
        &sessionAdapter, &infrastructure::SessionStateQtAdapter::testTimeModelChanged, this,
        [this](const presentation::viewModels::TestTimeViewModel & /*model*/) { shellPresenter.onStateChanged(); });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::functionExpressionChanged, this,
                     [this](const QString &expression) {
                         if (controlFormulaLineEdit == nullptr || controlFormulaLineEdit->hasFocus() ||
                             controlFormulaLineEdit->text() == expression) {
                             return;
                         }

                         controlFormulaLineEdit->setText(expression);
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
    const auto &stateData = sessionAdapter.getState().get();
    const domain::StandScenario scenario{stateData.standControlMode};
    if (!scenario.allowsManualImpact()) {
        const std::string message = "Manual stand control is disabled for this test mode";
        appendLog(message);
        showOperatorWarning("Ручное управление заблокировано", message);
        return;
    }

    domain::WindImpact target = domain::makeWindImpact(standBeaufortSpinBox->value(), selectedStandDirectionDegrees(),
                                                       standAngleOfAttackSpinBox->value());
    target.formula = stateData.functionExpression;

    setStandImpactUseCase.setTarget(target);
    standImpactTransitionTimer->start();

    appendLog("Manual stand impact target accepted: " + formatImpact(stateData.appliedStandImpact) + " -> " +
              formatImpact(target));

    if (scenario.manualImpactPolicy() == domain::ManualImpactPolicy::ReturnToScenarioAfterManualImpact) {
        appendLog("Hybrid stand mode accepted manual impact as a temporary override");
    }
}

void MainWindow::advanceStandImpactTransition() {
    const auto &stateData = sessionAdapter.getState().get();
    const domain::StandScenario scenario{stateData.standControlMode};
    if (!scenario.allowsManualImpact()) {
        standImpactTransitionTimer->stop();
        return;
    }

    const auto transition =
        domain::StandImpactTransition{}.advance(stateData.appliedStandImpact, stateData.targetStandImpact);
    const auto &next = transition.impact;

    setStandImpactUseCase.setApplied(next);

    controlChartsTabPresenter.onBeaufortChanged(next.beaufort.value());
    controlChartsTabPresenter.onAngleOfAttackChanged(next.angleOfAttack.degrees());
    controlChartsTabPresenter.onDirectionChanged(next.direction.degrees());
    controlChartsTabPresenter.onRebuildPlotPressed();

    if (transition.targetReached) {
        standImpactTransitionTimer->stop();
        appendLog("Manual stand impact target reached: " + formatImpact(next));

        if (scenario.manualImpactPolicy() == domain::ManualImpactPolicy::ReturnToScenarioAfterManualImpact) {
            appendLog("Hybrid scenario return is pending scenario engine implementation");
        }
    }
}

void MainWindow::selectTelemetryAxisColor() {
    const QColor color = QColorDialog::getColor(Qt::red, this);
    if (!color.isValid()) {
        return;
    }

    telemetryChartsTabPresenter.onTelemetryAxisColorSelected(selectedTelemetryAxisId(),
                                                             MainWindowUiAdapter::toDomainColor(color));
}

double MainWindow::selectedStandDirectionDegrees() const {
    return standDirectionComboBox->currentData().toDouble();
}

void MainWindow::updateControlFormulaTemplateSelection(const std::string &expression) {
    if (controlFormulaTemplateComboBox == nullptr) {
        return;
    }

    const QSignalBlocker blocker{controlFormulaTemplateComboBox};
    const auto key = domain::formulaTemplateKeyByExpression(expression);
    const int index = key.empty() ? 0
                                  : controlFormulaTemplateComboBox->findData(
                                        QString::fromUtf8(key.data(), static_cast<qsizetype>(key.size())));
    controlFormulaTemplateComboBox->setCurrentIndex(index >= 0 ? index : 0);
}

void MainWindow::updateManualStandControlsEnabled() {
    const auto mode = static_cast<domain::StandControlMode>(standControlModeComboBox->currentData().toInt());
    const bool manualEnabled = !domain::StandScenario{mode}.locksManualControls();

    standBeaufortSpinBox->setEnabled(manualEnabled);
    standAngleOfAttackSpinBox->setEnabled(manualEnabled);
    standDirectionComboBox->setEnabled(manualEnabled);
    standApplyButton->setEnabled(manualEnabled);
}

domain::AxisId MainWindow::selectedTelemetryAxisId() const {
    const int sourceIndex = telemetrySourceComboBox->currentData().toInt();
    return sourceIndex == 0 ? domain::axis0 : domain::axis1;
}

} // namespace ui
