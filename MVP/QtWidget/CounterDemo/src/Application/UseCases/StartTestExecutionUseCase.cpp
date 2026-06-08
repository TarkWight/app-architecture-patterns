#include "StartTestExecutionUseCase.hpp"

#include "../../Domain/AxisControlCommand.hpp"
#include "../../Domain/AxisId.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"
#include "../../Domain/WindControlProfileImpact.hpp"

namespace application::useCases {

StartTestExecutionUseCase::StartTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler,
    application::ports::ITelemetryClient &telemetryClient)
    : state(state), testExecutionScheduler(testExecutionScheduler), telemetryClient(telemetryClient) {
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
    applyScenarioImpact(0);

    testExecutionScheduler.start(0, [this](int elapsedSeconds) {
        state.setElapsedSeconds(elapsedSeconds);
        applyScenarioImpact(elapsedSeconds);

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

void StartTestExecutionUseCase::applyScenarioImpact(int elapsedSeconds) {
    const auto &session = state.get();
    if (session.testProtocol.testMode == domain::TestMode::Manual) {
        return;
    }

    const auto impact =
        domain::windImpactAt(session.controlProfile, domain::ElapsedSeconds::from(elapsedSeconds), session.windProfile);
    if (!impact.has_value()) {
        return;
    }

    state.setTargetStandImpact(*impact);
    state.setAppliedStandImpact(*impact);
    sendAppliedImpact(*impact);
}

void StartTestExecutionUseCase::sendAppliedImpact(const domain::WindProfile &profile) {
    telemetryClient.setAxisCommand(domain::axis0, domain::axis0WindCommand(profile));
    telemetryClient.setAxisCommand(domain::axis1, domain::axis1WindCommand(profile));
}

} // namespace application::useCases
