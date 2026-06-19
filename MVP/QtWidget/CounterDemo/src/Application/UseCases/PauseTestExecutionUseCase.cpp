#include "PauseTestExecutionUseCase.hpp"

#include "../../Domain/StandConnectionTransitions.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"

namespace application::useCases {

PauseTestExecutionUseCase::PauseTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler,
    application::ports::ITelemetryClient &telemetryClient)
    : state(state), testExecutionScheduler(testExecutionScheduler), telemetryClient(telemetryClient) {
}

PauseTestExecutionUseCase::PauseTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler,
    application::ports::ITelemetryClient &telemetryClient,
    application::services::TelemetrySessionClock &telemetrySessionClock)
    : state(state), testExecutionScheduler(testExecutionScheduler), telemetryClient(telemetryClient),
      telemetrySessionClock(&telemetrySessionClock) {
}

void PauseTestExecutionUseCase::execute() {
    const auto transition = domain::transitionAfterPauseRequested(state.get().execution.testExecutionStatus);
    if (!transition.has_value()) {
        return;
    }

    if (!testExecutionScheduler.isRunning()) {
        return;
    }

    testExecutionScheduler.pause();
    if (telemetrySessionClock != nullptr) {
        telemetrySessionClock->pause();
    }

    const auto pollingTransition = domain::transitionAfterPollingStopped(state.connection().standConnectionStatus);
    if (pollingTransition.has_value()) {
        telemetryClient.stopPolling();
        state.setStandConnectionStatus(*pollingTransition);
    }

    state.setTestExecutionStatus(*transition);
}

} // namespace application::useCases
