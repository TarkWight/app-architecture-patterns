#include "ResumeTestExecutionUseCase.hpp"

#include "../../Domain/StandConnectionTransitions.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"

namespace application::useCases {

ResumeTestExecutionUseCase::ResumeTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler,
    application::ports::ITelemetryClient &telemetryClient)
    : state(state), testExecutionScheduler(testExecutionScheduler), telemetryClient(telemetryClient) {
}

void ResumeTestExecutionUseCase::execute() {
    const auto transition = domain::transitionAfterResumeRequested(state.get().execution.testExecutionStatus);
    if (!transition.has_value()) {
        return;
    }

    if (!testExecutionScheduler.isPaused()) {
        return;
    }

    testExecutionScheduler.resume();

    const auto pollingTransition = domain::transitionAfterPollingStarted(state.connection().standConnectionStatus);
    if (pollingTransition.has_value()) {
        telemetryClient.startPolling(state.connection().telemetryPollInterval.milliseconds());
        state.setStandConnectionStatus(*pollingTransition);
    }

    state.setTestExecutionStatus(*transition);
}

} // namespace application::useCases
