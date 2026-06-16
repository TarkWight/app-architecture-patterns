#include "PauseTestExecutionUseCase.hpp"

#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"

namespace application::useCases {

PauseTestExecutionUseCase::PauseTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler)
    : state(state), testExecutionScheduler(testExecutionScheduler) {
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
    state.setTestExecutionStatus(*transition);
}

} // namespace application::useCases
