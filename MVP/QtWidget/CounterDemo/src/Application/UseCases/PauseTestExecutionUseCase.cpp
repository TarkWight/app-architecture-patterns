#include "PauseTestExecutionUseCase.hpp"

#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"

namespace application::useCases {

PauseTestExecutionUseCase::PauseTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler)
    : state(state), testExecutionScheduler(testExecutionScheduler) {
}

void PauseTestExecutionUseCase::execute() {
    if (!domain::canPause(state.get().testExecutionStatus)) {
        return;
    }

    if (!testExecutionScheduler.isRunning()) {
        return;
    }

    testExecutionScheduler.pause();
    state.setTestExecutionStatus(domain::TestExecutionStatus::Paused);
}

} // namespace application::useCases
