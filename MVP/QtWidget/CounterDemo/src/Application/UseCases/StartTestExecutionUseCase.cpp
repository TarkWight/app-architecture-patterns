#include "StartTestExecutionUseCase.hpp"

#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"

namespace application::useCases {

StartTestExecutionUseCase::StartTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler)
    : state(state), testExecutionScheduler(testExecutionScheduler) {
}

void StartTestExecutionUseCase::execute() {
    const auto &session = state.get();
    if (!domain::canStart(session.testExecutionStatus)) {
        return;
    }

    int activeDurationMinutes = 0;
    domain::TestTimeDirection direction = domain::TestTimeDirection::CountUp;

    if (session.testProtocol.testMode == domain::TestMode::Manual) {
        activeDurationMinutes = 0;
        direction = domain::TestTimeDirection::CountUp;
    } else {
        switch (session.testTimeSource) {
        case domain::TestTimeSource::AutoCalculated:
            activeDurationMinutes = session.estimatedTestDuration.value();
            direction = domain::TestTimeDirection::CountDown;
            break;

        case domain::TestTimeSource::OperatorDefined:
            activeDurationMinutes = session.operatorTestDuration.value();
            direction = domain::TestTimeDirection::CountDown;
            break;

        case domain::TestTimeSource::FreeRun:
            activeDurationMinutes = 0;
            direction = domain::TestTimeDirection::CountUp;
            break;
        }
    }

    state.setActiveTestDurationMinutes(activeDurationMinutes);
    state.setTestTimeDirection(direction);
    state.setElapsedSeconds(0);

    if (direction == domain::TestTimeDirection::CountDown) {
        state.setRemainingSeconds(activeDurationMinutes * 60);
    } else {
        state.setRemainingSeconds(0);
    }

    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);

    testExecutionScheduler.start(0, [this](int elapsedSeconds) {
        state.setElapsedSeconds(elapsedSeconds);

        const auto &current = state.get();

        if (current.testTimeDirection == domain::TestTimeDirection::CountDown) {
            const int totalSeconds = current.activeTestDuration.value() * 60;
            const int remainingSeconds = (elapsedSeconds < totalSeconds) ? (totalSeconds - elapsedSeconds) : 0;

            state.setRemainingSeconds(remainingSeconds);

            if (remainingSeconds == 0) {
                testExecutionScheduler.stop();
                state.setTestExecutionStatus(domain::TestExecutionStatus::Completed);
            }
        }
    });
}

} // namespace application::useCases
