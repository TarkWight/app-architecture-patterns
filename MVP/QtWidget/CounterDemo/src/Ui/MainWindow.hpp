#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>
#include <string>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/IShellView.hpp"
#include "../Presentation/ShellPresenter.hpp"
#include "../Presentation/TelemetryChartsTab/TelemetryChartsTabPresenter.hpp"
#include "../Presentation/ControlChartsTab/ControlChartsTabPresenter.hpp"
#include "../Presentation/TestProtocolTab/TestProtocolTabPresenter.hpp"
#include "../Application/UseCases/ApplyAngleOfAttackUseCase.hpp"
#include "../Application/UseCases/ApplyBeaufortImpactUseCase.hpp"
#include "../Application/UseCases/ApplyWindDirectionUseCase.hpp"
#include "../Application/UseCases/SetStandControlModeUseCase.hpp"
#include "../Application/UseCases/SetStandImpactUseCase.hpp"
#include "../Domain/AxisId.hpp"
#include "../Domain/TestTimeSource.hpp"
#include "../Infrastructure/ConfigTemplates/IConfigTemplateService.hpp"

class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QString;
class QTimer;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

namespace ui {

class TelemetryChartsTabWidget;
class ControlChartsTabWidget;
class TestProtocolTabWidget;

class MainWindow final : public QMainWindow, public presentation::IShellView {
    Q_OBJECT

  public:
    struct Dependencies {
        presentation::ShellPresenter &shellPresenter;
        presentation::telemetryChartsTab::TelemetryChartsTabPresenter &telemetryChartsTabPresenter;
        presentation::controlChartsTab::ControlChartsTabPresenter &controlChartsTabPresenter;
        presentation::testProtocolTab::TestProtocolTabPresenter &testProtocolTabPresenter;
        application::useCases::SetStandControlModeUseCase &setStandControlModeUseCase;
        application::useCases::SetStandImpactUseCase &setStandImpactUseCase;
        application::useCases::ApplyBeaufortImpactUseCase &applyBeaufortImpactUseCase;
        application::useCases::ApplyWindDirectionUseCase &applyWindDirectionUseCase;
        application::useCases::ApplyAngleOfAttackUseCase &applyAngleOfAttackUseCase;
        infrastructure::SessionStateQtAdapter &sessionAdapter;
        infrastructure::configTemplates::IConfigTemplateService &configTemplateService;
    };

    explicit MainWindow(Dependencies deps, QWidget *parent = nullptr);
    ~MainWindow() override;

    void setTimerText(const std::string &text) override;
    void setStartEnabled(bool enabled) override;
    void setStopEnabled(bool enabled) override;
    void setPauseResumeEnabled(bool enabled) override;
    void setPauseResumeText(const std::string &text) override;
    void setStandConnectionButtonText(const std::string &text) override;
    void setStandConnectionStatusText(const std::string &text) override;
    void setTestTimeSource(domain::TestTimeSource source) override;
    void setTestTimeSourceEnabled(bool enabled) override;

    void setFunctionExpression(const std::string &expression) override;

    void appendLog(const std::string &text) override;
    void showOperatorWarning(const std::string &title, const std::string &message) override;

  private:
    std::unique_ptr<Ui::MainWindow> ui;

    presentation::ShellPresenter &shellPresenter;
    presentation::telemetryChartsTab::TelemetryChartsTabPresenter &telemetryChartsTabPresenter;
    presentation::controlChartsTab::ControlChartsTabPresenter &controlChartsTabPresenter;
    presentation::testProtocolTab::TestProtocolTabPresenter &testProtocolTabPresenter;
    application::useCases::SetStandControlModeUseCase &setStandControlModeUseCase;
    application::useCases::SetStandImpactUseCase &setStandImpactUseCase;
    application::useCases::ApplyBeaufortImpactUseCase &applyBeaufortImpactUseCase;
    application::useCases::ApplyWindDirectionUseCase &applyWindDirectionUseCase;
    application::useCases::ApplyAngleOfAttackUseCase &applyAngleOfAttackUseCase;
    infrastructure::SessionStateQtAdapter &sessionAdapter;
    infrastructure::configTemplates::IConfigTemplateService &configTemplateService;

    TelemetryChartsTabWidget *telemetryChartsTabWidget{nullptr};
    ControlChartsTabWidget *controlChartsTabWidget{nullptr};
    TestProtocolTabWidget *testProtocolTabWidget{nullptr};

    QTimer *standImpactTransitionTimer{nullptr};
    bool controlPlotRebuildScheduled{false};
    std::string observedTestProtocolModeKey{};
    std::string selectedTelemetryConfigPath{};

    void setupTabs();
    void setupStandControlPanel();
    void connectShellSignals();
    void connectTestControlSignals();
    void connectConfigTemplateSignals();
    void connectStandControlSignals();
    void connectSessionSignals();
    void applyBeaufortImpact();
    void applyWindDirectionImpact();
    void applyAngleOfAttackImpact();
    void handleManualImpactAccepted(const domain::WindImpact &previousTarget, const std::string &changedParameter);
    void handleManualImpactRejected();
    void advanceStandImpactTransition();
    void selectTelemetryAxisColor();
    void scheduleControlPlotRebuild();
    void updateStandControlModeSelection();
    void updateManualStandControlsEnabled();
    void updateControlFormulaTemplateSelection(const std::string &expression);
    void createTelemetryTemplate();
    void selectTelemetryConfig();
    std::string telemetryConfigPathForConnection() const;
    double selectedStandDirectionDegrees() const;
    domain::AxisId selectedTelemetryAxisId() const;
};

} // namespace ui

#endif // MAINWINDOW_HPP
