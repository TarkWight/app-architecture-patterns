#include "ResumeTestExecutionUseCase.hpp"

#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"

namespace application::useCases {

ResumeTestExecutionUseCase::ResumeTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler)
    : state(state), testExecutionScheduler(testExecutionScheduler) {
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
    state.setTestExecutionStatus(*transition);
}

} // namespace application::useCases
