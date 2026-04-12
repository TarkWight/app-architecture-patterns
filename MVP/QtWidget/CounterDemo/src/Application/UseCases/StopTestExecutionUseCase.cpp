#include "StopTestExecutionUseCase.hpp"

#include "../../Domain/TestExecutionStatus.hpp"

namespace application::useCases {

StopTestExecutionUseCase::StopTestExecutionUseCase(
    application::session::SessionState &state,
    application::ports::ITestExecutionScheduler &testExecutionScheduler)
    : state(state), testExecutionScheduler(testExecutionScheduler) {
}

void StopTestExecutionUseCase::execute() {
    testExecutionScheduler.stop();
    state.setTestExecutionStatus(domain::TestExecutionStatus::Aborted);
}

} // namespace application::useCases