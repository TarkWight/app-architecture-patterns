#ifndef SHELLPRESENTER_HPP
#define SHELLPRESENTER_HPP

#include "../Application/Session/SessionState.hpp"
#include "../Application/UseCases/BuildControlPlotUseCase.hpp"
#include "../Application/UseCases/SetFunctionExpressionUseCase.hpp"
#include "../Application/UseCases/SetLineColorUseCase.hpp"
#include "../Application/UseCases/StartTestExecutionUseCase.hpp"
#include "../Application/UseCases/PauseTestExecutionUseCase.hpp"
#include "../Application/UseCases/ResumeTestExecutionUseCase.hpp"
#include "../Application/UseCases/StopTestExecutionUseCase.hpp"
#include "../Domain/Plot.hpp"
#include "../Domain/TestExecutionStatus.hpp"
#include "../Application/UseCases/SetTestTimeSourceUseCase.hpp"
#include "../Application/UseCases/ConfigureTelemetryUseCase.hpp"
#include "../Application/UseCases/ConnectStandUseCase.hpp"
#include "../Application/UseCases/DisconnectStandUseCase.hpp"
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
        application::useCases::BuildControlPlotUseCase &buildControlPlotUseCase;
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
    void onResumePressed();
    void onStopPressed();
    void onCalculatePressed();

    void onFunctionEdited(std::string expression);
    void onLineColorSelected(domain::RgbColor color);
    void onTestTimeSourceChanged(domain::TestTimeSource source);
    void onConnectTelemetryPressed(std::string configPath);

  private:
    application::session::SessionState &state;
    application::useCases::StartTestExecutionUseCase &startTestExecutionUseCase;
    application::useCases::PauseTestExecutionUseCase &pauseTestExecutionUseCase;
    application::useCases::ResumeTestExecutionUseCase &resumeTestExecutionUseCase;
    application::useCases::StopTestExecutionUseCase &stopTestExecutionUseCase;
    application::useCases::SetFunctionExpressionUseCase &setFunctionExpressionUseCase;
    application::useCases::SetLineColorUseCase &setLineColorUseCase;
    application::useCases::BuildControlPlotUseCase &buildControlPlotUseCase;
    application::useCases::SetTestTimeSourceUseCase &setTestTimeSourceUseCase;
    application::useCases::ConfigureTelemetryUseCase &configureTelemetryUseCase;
    application::useCases::ConnectStandUseCase &connectStandUseCase;
    application::useCases::DisconnectStandUseCase &disconnectStandUseCase;

    IShellView *view{nullptr};
    domain::StandConnectionStatus lastStandConnectionStatus{domain::StandConnectionStatus::Disconnected};

    static std::string formatTimerText(int secondsValue);
    static bool canStart(domain::TestExecutionStatus status);
    static bool canPause(domain::TestExecutionStatus status);
    static bool canResume(domain::TestExecutionStatus status);
    static bool canStop(domain::TestExecutionStatus status);

    void refreshFromState();
    void refreshStandConnectionButton();
    void notifyStandConnectionStatusChanged(domain::StandConnectionStatus status);
};

} // namespace presentation

#endif // SHELLPRESENTER_HPP
