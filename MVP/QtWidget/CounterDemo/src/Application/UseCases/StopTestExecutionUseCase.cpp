#include "StopTestExecutionUseCase.hpp"

#include "../../Domain/StandConnectionStatus.hpp"
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
    if (!domain::canStop(state.get().testExecutionStatus)) {
        return;
    }

    testExecutionScheduler.stop();

    if (state.get().standConnectionStatus == domain::StandConnectionStatus::Polling) {
        telemetryClient.stopPolling();
        state.setStandConnectionStatus(domain::StandConnectionStatus::Connected);
    }

    const auto &session = state.get();
    const auto stopPlan =
        domain::TestExecutionPlanner::resetAfterStop(session.activeTestDuration, session.testTimeDirection);

    state.setElapsedSeconds(stopPlan.elapsed);
    state.setRemainingSeconds(stopPlan.remaining);

    state.setTestExecutionStatus(domain::TestExecutionStatus::Ready);
}

} // namespace application::useCases
