#ifndef STARTTESTEXECUTIONUSECASE_HPP
#define STARTTESTEXECUTIONUSECASE_HPP

#include "BuildControlPlotUseCase.hpp"
#include "EstimateTestDurationUseCase.hpp"

#include "../Services/AppliedStandImpactSender.hpp"
#include "../Ports/ITelemetryClient.hpp"
#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Session/SessionState.hpp"

#include "../../Domain/Time.hpp"
#include "../../Domain/WindImpact.hpp"

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
    EstimateTestDurationUseCase estimateTestDurationUseCase;
    application::services::AppliedStandImpactSender appliedStandImpactSender;

    void startTelemetryPollingIfConnected();
    void stopTelemetryPollingIfActive();
    void applyScenarioImpact(domain::ElapsedSeconds elapsed);
};

} // namespace application::useCases

#endif // STARTTESTEXECUTIONUSECASE_HPP
