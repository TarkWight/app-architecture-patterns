#ifndef SHELLPRESENTER_HPP
#define SHELLPRESENTER_HPP

#include "../Application/Session/SessionState.hpp"
#include "../Application/UseCases/CalculateAndBuildControlPlotUseCase.hpp"
#include "../Application/UseCases/SetFunctionExpressionUseCase.hpp"
#include "../Application/UseCases/SetLineColorUseCase.hpp"
#include "../Application/UseCases/SetStandControlModeUseCase.hpp"
#include "../Application/UseCases/StartTestExecutionUseCase.hpp"
#include "../Application/UseCases/PauseTestExecutionUseCase.hpp"
#include "../Application/UseCases/ResumeTestExecutionUseCase.hpp"
#include "../Application/UseCases/StopTestExecutionUseCase.hpp"
#include "../Application/Dto/PlotModel.hpp"
#include "../Domain/TestExecutionStatus.hpp"
#include "../Application/UseCases/SetTestTimeSourceUseCase.hpp"
#include "../Application/UseCases/ConfigureTelemetryUseCase.hpp"
#include "../Application/UseCases/ConnectStandUseCase.hpp"
#include "../Application/UseCases/DisconnectStandUseCase.hpp"
#include "../Application/UseCases/EstimateTestDurationUseCase.hpp"
#include "../Domain/StandConnectionStatus.hpp"

#include "IShellView.hpp"

namespace presentation {

class ShellPresenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::StartTestExecutionUseCase &startTestExecutionUseCase;
        application::useCases::PauseTestExecutionUseCase &pauseTestExecutionUseCase;
        application::useCases::ResumeTestExecutionUseCase &resumeTestExecutionUseCase;
        application::useCases::StopTestExecutionUseCase &stopTestExecutionUseCase;
        application::useCases::SetTestTimeSourceUseCase &setTestTimeSourceUseCase;
        application::useCases::SetFunctionExpressionUseCase &setFunctionExpressionUseCase;
        application::useCases::SetLineColorUseCase &setLineColorUseCase;
        application::useCases::SetStandControlModeUseCase &setStandControlModeUseCase;
        application::useCases::CalculateAndBuildControlPlotUseCase &calculateAndBuildControlPlotUseCase;
        application::useCases::EstimateTestDurationUseCase &estimateTestDurationUseCase;
        application::useCases::ConfigureTelemetryUseCase &configureTelemetryUseCase;
        application::useCases::ConnectStandUseCase &connectStandUseCase;
        application::useCases::DisconnectStandUseCase &disconnectStandUseCase;
    };

    explicit ShellPresenter(Dependencies deps);

    void attachView(IShellView &view);
    void detachView();

    void onViewReady();
    void onStartPressed();
    void onStateChanged();
    void onPausePressed();
    void onPauseResumePressed();
    void onResumePressed();
    void onStopPressed();
    void onCalculatePressed();

    void onFunctionEdited(std::string expression);
    void onFormulaTemplateSelected(std::string key);
    void onLineColorSelected(application::dto::RgbColor color);
    void onTestTimeSourceChanged(domain::TestTimeSource source);
    void onStandControlModeChanged(domain::StandControlMode mode);
    void onConnectTelemetryPressed(std::string configPath);
    [[nodiscard]] bool telemetryConfigRequiredForConnection() const;

  private:
    application::session::SessionState &state;
    application::useCases::StartTestExecutionUseCase &startTestExecutionUseCase;
    application::useCases::PauseTestExecutionUseCase &pauseTestExecutionUseCase;
    application::useCases::ResumeTestExecutionUseCase &resumeTestExecutionUseCase;
    application::useCases::StopTestExecutionUseCase &stopTestExecutionUseCase;
    application::useCases::SetFunctionExpressionUseCase &setFunctionExpressionUseCase;
    application::useCases::SetLineColorUseCase &setLineColorUseCase;
    application::useCases::CalculateAndBuildControlPlotUseCase &calculateAndBuildControlPlotUseCase;
    application::useCases::EstimateTestDurationUseCase &estimateTestDurationUseCase;
    application::useCases::SetTestTimeSourceUseCase &setTestTimeSourceUseCase;
    application::useCases::ConfigureTelemetryUseCase &configureTelemetryUseCase;
    application::useCases::ConnectStandUseCase &connectStandUseCase;
    application::useCases::DisconnectStandUseCase &disconnectStandUseCase;
    application::useCases::SetStandControlModeUseCase &setStandControlModeUseCase;

    IShellView *view{nullptr};
    domain::StandConnectionStatus lastStandConnectionStatus{domain::StandConnectionStatus::Disconnected};
    bool standConnectionWarningShown{false};

    static std::string formatTimerText(int secondsValue);
    static bool canStart(domain::TestExecutionStatus status);
    static bool canPause(domain::TestExecutionStatus status);
    static bool canResume(domain::TestExecutionStatus status);
    static bool canStop(domain::TestExecutionStatus status);

    void refreshFromState();
    void refreshStandConnectionButton();
    void refreshStandConnectionStatusText();
    void notifyStandConnectionStatusChanged(domain::StandConnectionStatus status);
    [[nodiscard]] bool readinessAllowsStart();
    [[nodiscard]] bool connectionAllowsStart();
    [[nodiscard]] bool confirmDangerousReadinessStart(application::session::ReadinessStatus status);
    [[nodiscard]] static bool readinessGateRequired(domain::TestMode mode);
    [[nodiscard]] static bool readinessConfirmationRequired(application::session::ReadinessStatus status);
};

} // namespace presentation

#endif // SHELLPRESENTER_HPP
