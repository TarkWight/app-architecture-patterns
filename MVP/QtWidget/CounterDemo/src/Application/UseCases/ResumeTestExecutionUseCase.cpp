#include "ResumeTestExecutionUseCase.hpp"

#include "../../Domain/TestExecutionStatus.hpp"

namespace application::useCases {

ResumeTestExecutionUseCase::ResumeTestExecutionUseCase(
    application::session::SessionState &state,
    application::ports::ITestExecutionScheduler &testExecutionScheduler)
    : state(state), testExecutionScheduler(testExecutionScheduler) {
}

void ResumeTestExecutionUseCase::execute() {
    if (!testExecutionScheduler.isPaused()) {
        return;
    }

    testExecutionScheduler.resume();
    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
}

} // namespace application::useCases