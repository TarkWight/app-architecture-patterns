#include "StopTestExecutionUseCase.hpp"

#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/StandConnectionTransitions.hpp"
#include "../../Domain/TestExecutionPlanner.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"

namespace application::useCases {

StopTestExecutionUseCase::StopTestExecutionUseCase(application::session::SessionState &state,
                                                   application::ports::ITestExecutionScheduler &testExecutionScheduler,
                                                   application::ports::ITelemetryClient &telemetryClient)
    : state(state), testExecutionScheduler(testExecutionScheduler), telemetryClient(telemetryClient) {
}

StopTestExecutionUseCase::StopTestExecutionUseCase(application::session::SessionState &state,
                                                   application::ports::ITestExecutionScheduler &testExecutionScheduler,
                                                   application::ports::ITelemetryClient &telemetryClient,
                                                   application::services::TelemetrySessionClock &telemetrySessionClock)
    : state(state), testExecutionScheduler(testExecutionScheduler), telemetryClient(telemetryClient),
      telemetrySessionClock(&telemetrySessionClock) {
}

void StopTestExecutionUseCase::execute() {
    const auto transition = domain::transitionAfterStopRequested(state.execution().testExecutionStatus);
    if (!transition.has_value()) {
        return;
    }

    testExecutionScheduler.stop();
    if (telemetrySessionClock != nullptr) {
        telemetrySessionClock->pause();
    }

    const auto pollingTransition = domain::transitionAfterPollingStopped(state.connection().standConnectionStatus);
    if (pollingTransition.has_value()) {
        telemetryClient.stopPolling();
        state.setStandConnectionStatus(*pollingTransition);
    }

    const auto &execution = state.execution();
    const auto stopPlan =
        domain::TestExecutionPlanner::resetAfterStop(execution.activeTestDuration, execution.testTimeDirection);

    state.setElapsedSeconds(stopPlan.elapsed);
    state.setRemainingSeconds(stopPlan.remaining);

    state.setTestExecutionStatus(*transition);
}

} // namespace application::useCases
