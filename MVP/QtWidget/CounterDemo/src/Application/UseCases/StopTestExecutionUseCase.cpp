#include "StopTestExecutionUseCase.hpp"

#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"
#include "../../Domain/TestTimeDirection.hpp"

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

    state.setElapsedSeconds(0);

    if (session.testTimeDirection == domain::TestTimeDirection::CountDown) {
        state.setRemainingSeconds(session.activeTestDuration.value() * 60);
    } else {
        state.setRemainingSeconds(0);
    }

    state.setTestExecutionStatus(domain::TestExecutionStatus::Ready);
}

} // namespace application::useCases
