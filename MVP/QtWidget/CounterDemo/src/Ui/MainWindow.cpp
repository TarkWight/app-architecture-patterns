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

#include <QColorDialog>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QString>
#include <QStringList>
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

QString formatConfigPath(const std::filesystem::path &path) {
    return QString::fromStdString(path.string());
}

} // namespace

MainWindow::MainWindow(Dependencies deps, QWidget *parent)
    : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()), shellPresenter(deps.shellPresenter),
      telemetryChartsTabPresenter(deps.telemetryChartsTabPresenter),
      controlChartsTabPresenter(deps.controlChartsTabPresenter),
      testProtocolTabPresenter(deps.testProtocolTabPresenter),
      setStandControlModeUseCase(deps.setStandControlModeUseCase), setStandImpactUseCase(deps.setStandImpactUseCase),
      sessionAdapter(deps.sessionAdapter), configTemplateService(deps.configTemplateService) {
    ui->setupUi(this);

    ui->comboBoxTestTimeSource->addItem(QStringLiteral("Авторасчёт"));
    ui->comboBoxTestTimeSource->addItem(QStringLiteral("Время оператора"));
    ui->comboBoxTestTimeSource->addItem(QStringLiteral("Свободный режим"));

    standImpactTransitionTimer = new QTimer(this);
    standImpactTransitionTimer->setInterval(100);
    QObject::connect(standImpactTransitionTimer, &QTimer::timeout, this, &MainWindow::advanceStandImpactTransition);

    shellPresenter.attachView(*this);

    setupTabs();
    setupStandControlPanel();
    refreshConfigTemplateStatus();
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

void MainWindow::setupTabs() {
    telemetryChartsTabWidget = new TelemetryChartsTabWidget(telemetryChartsTabPresenter, sessionAdapter, this);
    controlChartsTabWidget = new ControlChartsTabWidget(controlChartsTabPresenter, sessionAdapter, this);
    testProtocolTabWidget = new TestProtocolTabWidget(testProtocolTabPresenter, sessionAdapter, this);

    ui->tabWidget->clear();
    ui->tabWidget->addTab(telemetryChartsTabWidget, QStringLiteral("Телеметрия"));
    ui->tabWidget->addTab(controlChartsTabWidget, QStringLiteral("Управляющие воздействия"));
    ui->tabWidget->addTab(testProtocolTabWidget, QStringLiteral("Протокол"));
}

void MainWindow::setupStandControlPanel() {
    ui->comboBoxStandControlMode->addItem(QStringLiteral("Ручной"), static_cast<int>(domain::StandControlMode::Manual));
    ui->comboBoxStandControlMode->addItem(QStringLiteral("Гибридный"),
                                          static_cast<int>(domain::StandControlMode::Hybrid));
    ui->comboBoxStandControlMode->addItem(QStringLiteral("Заготовленный"),
                                          static_cast<int>(domain::StandControlMode::PresetScenario));

    ui->doubleSpinBoxStandBeaufort->setRange(domain::minOperationalBeaufort, domain::maxOperationalBeaufort);
    ui->doubleSpinBoxStandAngleOfAttack->setRange(domain::minAngleOfAttack, domain::maxAngleOfAttack);

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
        ui->comboBoxStandDirection->addItem(
            QStringLiteral("%1 (%2°)").arg(QString::fromUtf8(label)).arg(degrees, 0, 'f', 1), degrees);
    }

    ui->comboBoxTelemetrySource->addItem(QStringLiteral("Ось Y / тангаж"), 0);
    ui->comboBoxTelemetrySource->addItem(QStringLiteral("Ось Z / направление"), 1);
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
    QObject::connect(controlChartsTabWidget, &ControlChartsTabWidget::formulaTemplateSelected, this,
                     [this](const QString &key) { shellPresenter.onFormulaTemplateSelected(key.toStdString()); });
    QObject::connect(controlChartsTabWidget, &ControlChartsTabWidget::calculateRequested, this,
                     [this]() { shellPresenter.onCalculatePressed(); });
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
        if (!configTemplateService.exists(configTemplates::ConfigTemplateType::Telemetry)) {
            refreshConfigTemplateStatus();

            const auto path = configTemplateService.pathFor(configTemplates::ConfigTemplateType::Telemetry);
            appendLog("telemetry.toml not found: " + path.string());
            showOperatorWarning("telemetry.toml не найден",
                                "Файл telemetry.toml не найден рядом с приложением. Создайте шаблон и заполните "
                                "параметры подключения стенда.");
            return;
        }

        shellPresenter.onConnectTelemetryPressed(
            configTemplateService.pathFor(configTemplates::ConfigTemplateType::Telemetry).string());
    });

    QObject::connect(ui->buttonCreateTelemetryTemplate, &QPushButton::clicked, this,
                     [this]() { createConfigTemplate(configTemplates::ConfigTemplateType::Telemetry); });

    QObject::connect(ui->buttonCreatePdfReportTemplate, &QPushButton::clicked, this,
                     [this]() { createConfigTemplate(configTemplates::ConfigTemplateType::PdfReport); });
}

void MainWindow::connectStandControlSignals() {
    QObject::connect(ui->comboBoxStandControlMode, &QComboBox::currentIndexChanged, this, [this]() {
        const auto mode = static_cast<domain::StandControlMode>(ui->comboBoxStandControlMode->currentData().toInt());
        setStandControlModeUseCase.execute(mode);
        updateManualStandControlsEnabled();
        scheduleControlPlotRebuild();
    });

    QObject::connect(ui->comboBoxTelemetrySource, &QComboBox::currentIndexChanged, this, [this](int index) {
        const QSignalBlocker visibleBlocker{ui->checkBoxTelemetryCurveVisible};

        const auto &stateData = sessionAdapter.getState().get();
        ui->checkBoxTelemetryCurveVisible->setChecked(index == 0 ? stateData.telemetryAxisYVisible
                                                                 : stateData.telemetryAxisZVisible);
    });

    QObject::connect(ui->checkBoxTelemetryCurveVisible, &QCheckBox::toggled, this, [this](bool checked) {
        telemetryChartsTabPresenter.onTelemetryAxisVisibilityChanged(selectedTelemetryAxisId(), checked);
    });

    QObject::connect(ui->buttonApplyStandImpact, &QPushButton::clicked, this, [this]() { applyStandInputs(); });
    QObject::connect(ui->buttonTelemetryLineColor, &QPushButton::clicked, this,
                     [this]() { selectTelemetryAxisColor(); });
}

void MainWindow::connectSessionSignals() {
    observedTestProtocolModeKey =
        std::string{domain::testModeKey(sessionAdapter.getState().get().testProtocol.testMode)};

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
                         updateStandControlModeSelection();
                         updateManualStandControlsEnabled();
                         scheduleControlPlotRebuild();
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

void MainWindow::applyStandInputs() {
    const auto &stateData = sessionAdapter.getState().get();
    const domain::StandScenario scenario{stateData.standControlMode};
    if (!scenario.allowsManualImpact()) {
        const std::string message = "Manual stand control is disabled for this test mode";
        appendLog(message);
        showOperatorWarning("Ручное управление заблокировано", message);
        return;
    }

    domain::WindImpact target =
        domain::makeWindImpact(ui->doubleSpinBoxStandBeaufort->value(), selectedStandDirectionDegrees(),
                               ui->doubleSpinBoxStandAngleOfAttack->value());

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

void MainWindow::scheduleControlPlotRebuild() {
    if (controlPlotRebuildScheduled) {
        return;
    }

    controlPlotRebuildScheduled = true;
    QTimer::singleShot(0, this, [this]() {
        controlPlotRebuildScheduled = false;
        controlChartsTabPresenter.onRebuildPlotPressed();
    });
}

double MainWindow::selectedStandDirectionDegrees() const {
    return ui->comboBoxStandDirection->currentData().toDouble();
}

void MainWindow::updateControlFormulaTemplateSelection(const std::string &expression) {
    controlChartsTabWidget->setFunctionExpression(expression);
}

void MainWindow::refreshConfigTemplateStatus() {
    const auto telemetryPath = configTemplateService.pathFor(configTemplates::ConfigTemplateType::Telemetry);
    const auto pdfReportPath = configTemplateService.pathFor(configTemplates::ConfigTemplateType::PdfReport);

    const bool telemetryExists = configTemplateService.exists(configTemplates::ConfigTemplateType::Telemetry);
    const bool pdfReportExists = configTemplateService.exists(configTemplates::ConfigTemplateType::PdfReport);

    ui->buttonCreateTelemetryTemplate->setEnabled(!telemetryExists);
    ui->buttonCreatePdfReportTemplate->setEnabled(!pdfReportExists);

    QStringList statusParts;
    statusParts << QStringLiteral("telemetry.toml: %1")
                       .arg(telemetryExists ? QStringLiteral("найден") : QStringLiteral("не найден"));
    statusParts << QStringLiteral("pdf_report.toml: %1")
                       .arg(pdfReportExists ? QStringLiteral("найден") : QStringLiteral("не найден"));

    ui->labelConfigTemplatesStatus->setText(statusParts.join(QStringLiteral("; ")));
    ui->labelConfigTemplatesStatus->setToolTip(
        QStringLiteral("telemetry.toml: %1\npdf_report.toml: %2")
            .arg(formatConfigPath(telemetryPath), formatConfigPath(pdfReportPath)));
}

void MainWindow::createConfigTemplate(configTemplates::ConfigTemplateType type) {
    try {
        configTemplateService.createTemplate(type);
        refreshConfigTemplateStatus();

        appendLog("Config template created or already exists: " + configTemplateService.pathFor(type).string());
    } catch (const std::exception &e) {
        appendLog(std::string{"Config template creation failed: "} + e.what());
        showOperatorWarning("Ошибка создания шаблона", e.what());
    }
}

void MainWindow::updateStandControlModeSelection() {
    const auto mode = sessionAdapter.getState().get().standControlMode;
    const int index = ui->comboBoxStandControlMode->findData(static_cast<int>(mode));
    if (index < 0 || ui->comboBoxStandControlMode->currentIndex() == index) {
        return;
    }

    const QSignalBlocker blocker{ui->comboBoxStandControlMode};
    ui->comboBoxStandControlMode->setCurrentIndex(index);
}

void MainWindow::updateManualStandControlsEnabled() {
    const auto mode = static_cast<domain::StandControlMode>(ui->comboBoxStandControlMode->currentData().toInt());
    const bool manualEnabled = !domain::StandScenario{mode}.locksManualControls();

    ui->doubleSpinBoxStandBeaufort->setEnabled(manualEnabled);
    ui->doubleSpinBoxStandAngleOfAttack->setEnabled(manualEnabled);
    ui->comboBoxStandDirection->setEnabled(manualEnabled);
    ui->buttonApplyStandImpact->setEnabled(manualEnabled);
}

domain::AxisId MainWindow::selectedTelemetryAxisId() const {
    const int sourceIndex = ui->comboBoxTelemetrySource->currentData().toInt();
    return sourceIndex == 0 ? domain::axis0 : domain::axis1;
}

} // namespace ui
