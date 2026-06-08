#ifndef STARTTESTEXECUTIONUSECASE_HPP
#define STARTTESTEXECUTIONUSECASE_HPP

#include "BuildControlPlotUseCase.hpp"

#include "../Ports/ITelemetryClient.hpp"
#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class StartTestExecutionUseCase final {
  public:
    StartTestExecutionUseCase(application::session::SessionState &state,
                              application::ports::ITestExecutionScheduler &testExecutionScheduler,
                              application::ports::ITelemetryClient &telemetryClient,
                              BuildControlPlotUseCase &buildControlPlotUseCase);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITestExecutionScheduler &testExecutionScheduler;
    application::ports::ITelemetryClient &telemetryClient;
    BuildControlPlotUseCase &buildControlPlotUseCase;

    void startTelemetryPollingIfConnected();
    void stopTelemetryPollingIfActive();
    void applyScenarioImpact(int elapsedSeconds);
    void sendAppliedImpact(const domain::WindImpact &profile);
};

} // namespace application::useCases

#endif // STARTTESTEXECUTIONUSECASE_HPP
