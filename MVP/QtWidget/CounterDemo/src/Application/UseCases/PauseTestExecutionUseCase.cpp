#include "PauseTestExecutionUseCase.hpp"

#include "../../Domain/TestExecutionStatus.hpp"

namespace application::useCases {

PauseTestExecutionUseCase::PauseTestExecutionUseCase(
    application::session::SessionState &state,
    application::ports::ITestExecutionScheduler &testExecutionScheduler)
    : state(state), testExecutionScheduler(testExecutionScheduler) {
}

void PauseTestExecutionUseCase::execute() {
    if (!testExecutionScheduler.isRunning()) {
        return;
    }

    testExecutionScheduler.pause();
    state.setTestExecutionStatus(domain::TestExecutionStatus::Paused);
}

} // namespace application::useCases