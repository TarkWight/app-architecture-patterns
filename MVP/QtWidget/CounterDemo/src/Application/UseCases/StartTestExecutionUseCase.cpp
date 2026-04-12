#include "StartTestExecutionUseCase.hpp"

#include "../../Domain/TestExecutionStatus.hpp"

namespace application::useCases {

StartTestExecutionUseCase::StartTestExecutionUseCase(
    application::session::SessionState &state,
    application::ports::ITestExecutionScheduler &testExecutionScheduler)
    : state(state), testExecutionScheduler(testExecutionScheduler) {
}

void StartTestExecutionUseCase::execute() {
    state.setElapsedSeconds(0);
    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);

    testExecutionScheduler.start(0, [this](int elapsedSeconds) {
        state.setElapsedSeconds(elapsedSeconds);
    });
}

} // namespace application::useCases