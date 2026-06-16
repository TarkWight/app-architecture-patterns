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

void StopTestExecutionUseCase::execute() {
    const auto transition = domain::transitionAfterStopRequested(state.get().execution.testExecutionStatus);
    if (!transition.has_value()) {
        return;
    }

    testExecutionScheduler.stop();

    const auto pollingTransition = domain::transitionAfterPollingStopped(state.get().connection.standConnectionStatus);
    if (pollingTransition.has_value()) {
        telemetryClient.stopPolling();
        state.setStandConnectionStatus(*pollingTransition);
    }

    const auto &session = state.get();
    const auto stopPlan = domain::TestExecutionPlanner::resetAfterStop(session.execution.activeTestDuration,
                                                                       session.execution.testTimeDirection);

    state.setElapsedSeconds(stopPlan.elapsed);
    state.setRemainingSeconds(stopPlan.remaining);

    state.setTestExecutionStatus(*transition);
}

} // namespace application::useCases
