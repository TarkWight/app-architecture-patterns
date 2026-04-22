#include "StopTestExecutionUseCase.hpp"

#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestTimeDirection.hpp"

namespace application::useCases {

StopTestExecutionUseCase::StopTestExecutionUseCase(
    application::session::SessionState &state,
    application::ports::ITestExecutionScheduler &testExecutionScheduler)
    : state(state), testExecutionScheduler(testExecutionScheduler) {
}

void StopTestExecutionUseCase::execute() {
    testExecutionScheduler.stop();

    const auto &session = state.get();

    state.setElapsedSeconds(0);

    if (session.testTimeDirection == domain::TestTimeDirection::CountDown) {
        state.setRemainingSeconds(session.activeTestDuration.value * 60);
    } else {
        state.setRemainingSeconds(0);
    }

    state.setTestExecutionStatus(domain::TestExecutionStatus::Ready);
}

} // namespace application::useCases