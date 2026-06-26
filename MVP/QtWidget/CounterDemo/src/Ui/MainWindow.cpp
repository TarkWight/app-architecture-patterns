#include "MainWindow.hpp"

#include "../Presentation/ViewModels/TestTimeViewModel.hpp"
#include "../Localization/UiStrings.hpp"
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

#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QString>
#include <QTimer>

#include <array>
#include <cmath>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <utility>

namespace ui {

namespace {
namespace configTemplates = infrastructure::configTemplates;

std::string compassLabel(double degrees) {
    const auto normalized = std::fmod(degrees + 360.0, 360.0);
    const auto index =
        static_cast<std::size_t>(std::lround(normalized / 22.5)) % localization::ui::compassDirectionLabels.size();
    return localization::ui::compassDirectionLabels[index];
}

std::string formatImpact(const domain::WindImpact &profile) {
    std::ostringstream beaufort;
    beaufort << std::fixed << std::setprecision(1) << profile.beaufort.value();

    std::ostringstream direction;
    direction << std::fixed << std::setprecision(1) << profile.direction.degrees();

    std::ostringstream angleOfAttack;
    angleOfAttack << std::fixed << std::setprecision(1) << profile.angleOfAttack.degrees();

    return localization::ui::formattedImpact(beaufort.str(), compassLabel(profile.direction.degrees()), direction.str(),
                                             angleOfAttack.str());
}

QString uiText(const char *text) {
    return QString::fromUtf8(text);
}

} // namespace

MainWindow::MainWindow(Dependencies deps, QWidget *parent)
    : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()), shellPresenter(deps.shellPresenter),
      telemetryChartsTabPresenter(deps.telemetryChartsTabPresenter),
      controlChartsTabPresenter(deps.controlChartsTabPresenter),
      testProtocolTabPresenter(deps.testProtocolTabPresenter), setStandImpactUseCase(deps.setStandImpactUseCase),
      applyBeaufortImpactUseCase(deps.applyBeaufortImpactUseCase),
      applyWindDirectionUseCase(deps.applyWindDirectionUseCase),
      applyAngleOfAttackUseCase(deps.applyAngleOfAttackUseCase), sessionAdapter(deps.sessionAdapter),
      configTemplateService(deps.configTemplateService) {
    ui->setupUi(this);

    ui->comboBoxTestTimeSource->addItem(uiText(localization::ui::testTimeSourceAutoCalculated));
    ui->comboBoxTestTimeSource->addItem(uiText(localization::ui::testTimeSourceOperatorDefined));
    ui->comboBoxTestTimeSource->addItem(uiText(localization::ui::testTimeSourceFreeRun));
    ui->buttonCreateTelemetryTemplate->setText(uiText(localization::ui::createTelemetryTemplateDialogTitle));
    ui->buttonSelectTelemetryConfig->setText(uiText(localization::ui::selectTelemetryConfigDialogTitle));

    standImpactTransitionTimer = new QTimer(this);
    standImpactTransitionTimer->setInterval(100);
    QObject::connect(standImpactTransitionTimer, &QTimer::timeout, this, &MainWindow::advanceStandImpactTransition);

    shellPresenter.attachView(*this);

    setupTabs();
    setupStandControlPanel();
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

void MainWindow::setPauseResumeEnabled(bool enabled) {
    ui->buttonPause->setEnabled(enabled);
}

void MainWindow::setPauseResumeText(const std::string &text) {
    ui->buttonPause->setText(QString::fromStdString(text));
}

void MainWindow::setStandConnectionButtonText(const std::string &text) {
    ui->buttonConnectTelemetry->setText(QString::fromStdString(text));
}

void MainWindow::setStandConnectionStatusText(const std::string &text) {
    ui->labelStandConnectionStatus->setText(QString::fromStdString(text));
}

void MainWindow::setFunctionExpression(const std::string &expression) {
    controlChartsTabWidget->setFunctionExpression(expression);
}

void MainWindow::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void MainWindow::showOperatorWarning(const std::string &title, const std::string &message) {
    QMessageBox::warning(this, QString::fromStdString(title), QString::fromStdString(message));
}

bool MainWindow::confirmDangerousReadinessStart(const std::string &title, const std::string &message) {
    return QMessageBox::warning(this, QString::fromStdString(title), QString::fromStdString(message),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

void MainWindow::freezeStandImpactTransition() {
    standImpactTransitionTimer->stop();
}

void MainWindow::setupTabs() {
    telemetryChartsTabWidget = new TelemetryChartsTabWidget(telemetryChartsTabPresenter, sessionAdapter, this);
    controlChartsTabWidget = new ControlChartsTabWidget(controlChartsTabPresenter, sessionAdapter, this);
    testProtocolTabWidget = new TestProtocolTabWidget(testProtocolTabPresenter, sessionAdapter, this);

    ui->tabWidget->clear();
    ui->tabWidget->addTab(telemetryChartsTabWidget, uiText(localization::ui::tabTelemetry));
    ui->tabWidget->addTab(controlChartsTabWidget, uiText(localization::ui::tabControlCharts));
    ui->tabWidget->addTab(testProtocolTabWidget, uiText(localization::ui::tabProtocol));
}

void MainWindow::setupStandControlPanel() {
    ui->doubleSpinBoxStandBeaufort->setRange(domain::minOperationalBeaufort, domain::maxOperationalBeaufort);
    ui->doubleSpinBoxStandAngleOfAttack->setRange(domain::minAngleOfAttack, domain::maxAngleOfAttack);

    const std::array<std::pair<const char *, double>, 16> directions{{
        {localization::ui::compassDirectionLabels[0], 0.0},
        {localization::ui::compassDirectionLabels[1], 22.5},
        {localization::ui::compassDirectionLabels[2], 45.0},
        {localization::ui::compassDirectionLabels[3], 67.5},
        {localization::ui::compassDirectionLabels[4], 90.0},
        {localization::ui::compassDirectionLabels[5], 112.5},
        {localization::ui::compassDirectionLabels[6], 135.0},
        {localization::ui::compassDirectionLabels[7], 157.5},
        {localization::ui::compassDirectionLabels[8], 180.0},
        {localization::ui::compassDirectionLabels[9], 202.5},
        {localization::ui::compassDirectionLabels[10], 225.0},
        {localization::ui::compassDirectionLabels[11], 247.5},
        {localization::ui::compassDirectionLabels[12], 270.0},
        {localization::ui::compassDirectionLabels[13], 292.5},
        {localization::ui::compassDirectionLabels[14], 315.0},
        {localization::ui::compassDirectionLabels[15], 337.5},
    }};

    for (const auto &[label, degrees] : directions) {
        ui->comboBoxStandDirection->addItem(
            QStringLiteral("%1 (%2°)").arg(QString::fromUtf8(label)).arg(degrees, 0, 'f', 1), degrees);
    }

    ui->comboBoxTelemetrySource->addItem(uiText(localization::ui::telemetryAxisY), 0);
    ui->comboBoxTelemetrySource->addItem(uiText(localization::ui::telemetryAxisZ), 1);
}

void MainWindow::connectShellSignals() {
    connectTestControlSignals();
    connectConfigTemplateSignals();
    connectStandControlSignals();
}

void MainWindow::connectTestControlSignals() {
    QObject::connect(ui->buttonStart, &QPushButton::clicked, this, [this]() { shellPresenter.onStartPressed(); });

    QObject::connect(ui->buttonPause, &QPushButton::clicked, this, [this]() { shellPresenter.onPauseResumePressed(); });

    QObject::connect(ui->buttonStop, &QPushButton::clicked, this, [this]() { shellPresenter.onStopPressed(); });

    QObject::connect(controlChartsTabWidget, &ControlChartsTabWidget::functionEdited, this,
                     [this](const QString &expression) { shellPresenter.onFunctionEdited(expression.toStdString()); });
    QObject::connect(controlChartsTabWidget, &ControlChartsTabWidget::calculateRequested, this, [this]() {
        shellPresenter.onCalculatePressed();
        controlChartsTabPresenter.onCalculationResultChanged();
    });
    QObject::connect(controlChartsTabWidget, &ControlChartsTabWidget::lineColorRequested, this, [this]() {
        const QColor color = QColorDialog::getColor(Qt::red, this);
        if (!color.isValid()) {
            return;
        }

        shellPresenter.onLineColorSelected(MainWindowUiAdapter::toDomainColor(color));
    });
    QObject::connect(controlChartsTabWidget, &ControlChartsTabWidget::logMessage, this,
                     [this](const QString &text) { appendLog(text.toStdString()); });
    QObject::connect(telemetryChartsTabWidget, &TelemetryChartsTabWidget::logMessage, this,
                     [this](const QString &text) { appendLog(text.toStdString()); });

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
}

void MainWindow::connectConfigTemplateSignals() {
    QObject::connect(ui->buttonConnectTelemetry, &QPushButton::clicked, this, [this]() {
        if (!shellPresenter.telemetryConfigRequiredForConnection()) {
            shellPresenter.onConnectTelemetryPressed({});
            return;
        }

        const auto telemetryConfig = configTemplateService.resolvePath(configTemplates::ConfigTemplateType::Telemetry,
                                                                       selectedTelemetryConfigPath);

        if (!telemetryConfig.exists()) {
            if (telemetryConfig.status == configTemplates::ConfigTemplateResolutionStatus::MissingOperatorSelected) {
                appendLog(std::string{localization::ui::unavailableSelectedTelemetryConfigLog} +
                          telemetryConfig.path.string());
                showOperatorWarning(localization::ui::telemetryConfigUnavailableTitle,
                                    localization::ui::telemetryConfigUnavailableMessage);
                return;
            }

            appendLog(std::string{localization::ui::telemetryConfigNotFoundLog} + telemetryConfig.path.string());
            showOperatorWarning(localization::ui::telemetryConfigNotFoundTitle,
                                localization::ui::telemetryConfigNotFoundMessage);
            return;
        }

        shellPresenter.onConnectTelemetryPressed(telemetryConfig.path.string());
    });

    QObject::connect(ui->buttonCreateTelemetryTemplate, &QPushButton::clicked, this,
                     [this]() { createTelemetryTemplate(); });

    QObject::connect(ui->buttonSelectTelemetryConfig, &QPushButton::clicked, this,
                     [this]() { selectTelemetryConfig(); });
}

void MainWindow::connectStandControlSignals() {
    QObject::connect(ui->comboBoxTelemetrySource, &QComboBox::currentIndexChanged, this, [this](int index) {
        const QSignalBlocker visibleBlocker{ui->checkBoxTelemetryCurveVisible};

        const auto &stateData = sessionAdapter.getState().get();
        ui->checkBoxTelemetryCurveVisible->setChecked(index == 0 ? stateData.telemetry.telemetryAxisYVisible
                                                                 : stateData.telemetry.telemetryAxisZVisible);
    });

    QObject::connect(ui->checkBoxTelemetryCurveVisible, &QCheckBox::toggled, this, [this](bool checked) {
        telemetryChartsTabPresenter.onTelemetryAxisVisibilityChanged(selectedTelemetryAxisId(), checked);
    });

    QObject::connect(ui->buttonApplyStandBeaufort, &QPushButton::clicked, this, [this]() { applyBeaufortImpact(); });
    QObject::connect(ui->buttonApplyStandDirection, &QPushButton::clicked, this,
                     [this]() { applyWindDirectionImpact(); });
    QObject::connect(ui->buttonApplyStandAngleOfAttack, &QPushButton::clicked, this,
                     [this]() { applyAngleOfAttackImpact(); });
    QObject::connect(ui->buttonTelemetryLineColor, &QPushButton::clicked, this,
                     [this]() { selectTelemetryAxisColor(); });
}

void MainWindow::connectSessionSignals() {
    observedTestProtocolModeKey =
        std::string{domain::testModeKey(sessionAdapter.getState().get().protocol.testProtocol.testMode)};

    QObject::connect(
        &sessionAdapter, &infrastructure::SessionStateQtAdapter::testTimeModelChanged, this,
        [this](const presentation::viewModels::TestTimeViewModel & /*model*/) { shellPresenter.onStateChanged(); });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::functionExpressionChanged, this,
                     [this](const QString &expression) { setFunctionExpression(expression.toStdString()); });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::testProtocolModeChanged, this,
                     [this](const QString &mode) {
                         const std::string modeKey = mode.toStdString();
                         if (modeKey == observedTestProtocolModeKey) {
                             return;
                         }

                         observedTestProtocolModeKey = modeKey;
                         updateManualStandControlsEnabled();
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

    if (ui->comboBoxTestTimeSource->currentIndex() == index) {
        return;
    }

    const QSignalBlocker blocker{ui->comboBoxTestTimeSource};
    ui->comboBoxTestTimeSource->setCurrentIndex(index);
}

void MainWindow::setTestTimeSourceEnabled(bool enabled) {
    ui->comboBoxTestTimeSource->setEnabled(enabled);
}

void MainWindow::applyBeaufortImpact() {
    const auto previousTarget = sessionAdapter.getState().get().control.targetStandImpact;
    if (!applyBeaufortImpactUseCase.execute(domain::Beaufort::from(ui->doubleSpinBoxStandBeaufort->value()))) {
        handleManualImpactRejected();
        return;
    }

    handleManualImpactAccepted(previousTarget, localization::ui::beaufortParameter);
}

void MainWindow::applyWindDirectionImpact() {
    const auto previousTarget = sessionAdapter.getState().get().control.targetStandImpact;
    if (!applyWindDirectionUseCase.execute(domain::WindDirection::from(selectedStandDirectionDegrees()))) {
        handleManualImpactRejected();
        return;
    }

    handleManualImpactAccepted(previousTarget, localization::ui::directionParameter);
}

void MainWindow::applyAngleOfAttackImpact() {
    const auto previousTarget = sessionAdapter.getState().get().control.targetStandImpact;
    if (!applyAngleOfAttackUseCase.execute(domain::AngleOfAttack::from(ui->doubleSpinBoxStandAngleOfAttack->value()))) {
        handleManualImpactRejected();
        return;
    }

    handleManualImpactAccepted(previousTarget, localization::ui::angleOfAttackParameter);
}

void MainWindow::handleManualImpactAccepted(const domain::WindImpact &previousTarget,
                                            const std::string &changedParameter) {
    const auto &stateData = sessionAdapter.getState().get();
    const domain::StandScenario scenario{stateData.control.standControlMode};
    standImpactTransitionTimer->start();

    appendLog(localization::ui::manualImpactAcceptedLog(changedParameter, formatImpact(previousTarget),
                                                        formatImpact(stateData.control.targetStandImpact),
                                                        formatImpact(stateData.control.appliedStandImpact)));

    if (scenario.manualImpactPolicy() == domain::ManualImpactPolicy::ReturnToScenarioAfterManualImpact) {
        appendLog(localization::ui::hybridManualOverrideAccepted);
    }
}

void MainWindow::handleManualImpactRejected() {
    const std::string message = localization::ui::manualControlBlockedMessage;
    appendLog(message);
    showOperatorWarning(localization::ui::manualControlBlockedTitle, message);
}

void MainWindow::advanceStandImpactTransition() {
    const auto &stateData = sessionAdapter.getState().get();
    const domain::StandScenario scenario{stateData.control.standControlMode};
    if (!scenario.allowsManualImpact()) {
        standImpactTransitionTimer->stop();
        return;
    }

    const auto transition = domain::StandImpactTransition{}.advance(stateData.control.appliedStandImpact,
                                                                    stateData.control.targetStandImpact);
    const auto &next = transition.impact;

    setStandImpactUseCase.setApplied(next);

    controlChartsTabPresenter.onBeaufortChanged(next.beaufort.value());
    controlChartsTabPresenter.onAngleOfAttackChanged(next.angleOfAttack.degrees());
    controlChartsTabPresenter.onDirectionChanged(next.direction.degrees());
    controlChartsTabPresenter.onRebuildPlotPressed();

    if (transition.targetReached) {
        standImpactTransitionTimer->stop();
        appendLog(localization::ui::manualImpactTargetReachedLog(formatImpact(next)));
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
    return ui->comboBoxStandDirection->currentData().toDouble();
}

void MainWindow::updateControlFormulaTemplateSelection(const std::string &expression) {
    controlChartsTabWidget->setFunctionExpression(expression);
}

void MainWindow::createTelemetryTemplate() {
    const auto defaultPath = configTemplateService.pathFor(configTemplates::ConfigTemplateType::Telemetry);
    const QString filePath = QFileDialog::getSaveFileName(
        this, uiText(localization::ui::createTelemetryTemplateDialogTitle),
        QString::fromStdString(defaultPath.string()), uiText(localization::ui::tomlFileFilter));

    if (filePath.isEmpty()) {
        return;
    }

    try {
        configTemplateService.createTemplate(configTemplates::ConfigTemplateType::Telemetry,
                                             std::filesystem::path{filePath.toStdString()});
        appendLog(std::string{localization::ui::telemetryTemplateCreatedLog} + filePath.toStdString());
    } catch (const std::exception &e) {
        appendLog(std::string{localization::ui::telemetryTemplateCreationFailedLog} + e.what());
        showOperatorWarning(localization::ui::templateCreationErrorTitle, e.what());
    }
}

void MainWindow::selectTelemetryConfig() {
    const auto defaultPath = configTemplateService.pathFor(configTemplates::ConfigTemplateType::Telemetry);
    const QString filePath = QFileDialog::getOpenFileName(
        this, uiText(localization::ui::selectTelemetryConfigDialogTitle), QString::fromStdString(defaultPath.string()),
        uiText(localization::ui::tomlFileFilter));

    if (filePath.isEmpty()) {
        return;
    }

    selectedTelemetryConfigPath = filePath.toStdString();
    appendLog(std::string{localization::ui::telemetryConfigSelectedLog} + selectedTelemetryConfigPath);
}

void MainWindow::updateManualStandControlsEnabled() {
    const auto mode = sessionAdapter.getState().control().standControlMode;
    const bool manualEnabled = !domain::StandScenario{mode}.locksManualControls();

    ui->doubleSpinBoxStandBeaufort->setEnabled(manualEnabled);
    ui->doubleSpinBoxStandAngleOfAttack->setEnabled(manualEnabled);
    ui->comboBoxStandDirection->setEnabled(manualEnabled);
    ui->buttonApplyStandBeaufort->setEnabled(manualEnabled);
    ui->buttonApplyStandDirection->setEnabled(manualEnabled);
    ui->buttonApplyStandAngleOfAttack->setEnabled(manualEnabled);
}

domain::AxisId MainWindow::selectedTelemetryAxisId() const {
    const int sourceIndex = ui->comboBoxTelemetrySource->currentData().toInt();
    return sourceIndex == 0 ? domain::axis0 : domain::axis1;
}

} // namespace ui
