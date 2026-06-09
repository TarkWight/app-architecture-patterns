#include "ResumeTestExecutionUseCase.hpp"

#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"

namespace application::useCases {

ResumeTestExecutionUseCase::ResumeTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler)
    : state(state), testExecutionScheduler(testExecutionScheduler) {
}

void ResumeTestExecutionUseCase::execute() {
    if (!domain::canResume(state.get().testExecutionStatus)) {
        return;
    }

    if (!testExecutionScheduler.isPaused()) {
        return;
    }

    testExecutionScheduler.resume();
    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
}

} // namespace application::useCases
