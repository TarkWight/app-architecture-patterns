#ifndef STARTTESTEXECUTIONUSECASE_HPP
#define STARTTESTEXECUTIONUSECASE_HPP

#include "BuildControlPlotUseCase.hpp"
#include "EstimateTestDurationUseCase.hpp"

#include "../Services/AppliedStandImpactSender.hpp"
#include "../Services/TelemetrySessionClock.hpp"
#include "../Ports/ITelemetryClient.hpp"
#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Session/SessionState.hpp"

#include "../../Domain/Time.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::useCases {

class StartTestExecutionUseCase final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::ports::ITestExecutionScheduler &testExecutionScheduler;
        application::ports::ITelemetryClient &telemetryClient;
        BuildControlPlotUseCase &buildControlPlotUseCase;
        application::services::TelemetrySessionClock &telemetrySessionClock;
    };

    StartTestExecutionUseCase(application::session::SessionState &state,
                              application::ports::ITestExecutionScheduler &testExecutionScheduler,
                              application::ports::ITelemetryClient &telemetryClient,
                              BuildControlPlotUseCase &buildControlPlotUseCase);
    explicit StartTestExecutionUseCase(Dependencies deps);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITestExecutionScheduler &testExecutionScheduler;
    application::ports::ITelemetryClient &telemetryClient;
    BuildControlPlotUseCase &buildControlPlotUseCase;
    EstimateTestDurationUseCase estimateTestDurationUseCase;
    application::services::AppliedStandImpactSender appliedStandImpactSender;
    application::services::TelemetrySessionClock *telemetrySessionClock{nullptr};

    void startTelemetryPollingIfConnected();
    void stopTelemetryPollingIfActive();
    void applyScenarioImpact(domain::ElapsedSeconds elapsed);
};

} // namespace application::useCases

#endif // STARTTESTEXECUTIONUSECASE_HPP
