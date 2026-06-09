#include "StartTestExecutionUseCase.hpp"

#include "../../Domain/AxisControlCommand.hpp"
#include "../../Domain/AxisId.hpp"
#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"
#include "../../Domain/StandImpactTransition.hpp"
#include "../../Domain/WindControlProfileImpact.hpp"

namespace application::useCases {

StartTestExecutionUseCase::StartTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler,
    application::ports::ITelemetryClient &telemetryClient, BuildControlPlotUseCase &buildControlPlotUseCase)
    : state(state), testExecutionScheduler(testExecutionScheduler), telemetryClient(telemetryClient),
      buildControlPlotUseCase(buildControlPlotUseCase) {
}

void StartTestExecutionUseCase::execute() {
    const auto &session = state.get();
    if (!domain::canStart(session.testExecutionStatus)) {
        return;
    }

    if (session.testProtocol.testMode != domain::TestMode::Manual) {
        buildControlPlotUseCase.execute();
        state.setTargetStandImpact(state.get().windImpact);
        state.clearControlTrace();
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
    startTelemetryPollingIfConnected();
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
                stopTelemetryPollingIfActive();
                state.setTestExecutionStatus(domain::TestExecutionStatus::Completed);
            }
        }
    });
}

void StartTestExecutionUseCase::startTelemetryPollingIfConnected() {
    if (state.get().standConnectionStatus != domain::StandConnectionStatus::Connected) {
        return;
    }

    telemetryClient.startPolling(state.get().telemetryPollIntervalMs);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Polling);
}

void StartTestExecutionUseCase::stopTelemetryPollingIfActive() {
    if (state.get().standConnectionStatus != domain::StandConnectionStatus::Polling) {
        return;
    }

    telemetryClient.stopPolling();
    state.setStandConnectionStatus(domain::StandConnectionStatus::Connected);
}

void StartTestExecutionUseCase::applyScenarioImpact(int elapsedSeconds) {
    const auto &session = state.get();
    if (session.testProtocol.testMode == domain::TestMode::Manual) {
        return;
    }

    const auto impact = domain::windImpactAt(session.controlProfile, domain::ElapsedSeconds::from(elapsedSeconds),
                                             session.targetStandImpact);
    if (!impact.has_value()) {
        return;
    }

    const auto transition = domain::StandImpactTransition{}.advance(session.appliedStandImpact, *impact);

    state.setTargetStandImpact(*impact);
    state.setAppliedStandImpact(transition.impact);
    state.appendControlTraceSample(domain::ControlTraceSample{.timeSeconds = static_cast<double>(elapsedSeconds),
                                                              .targetValue = *impact,
                                                              .safeCommandValue = transition.impact});
    buildControlPlotUseCase.refreshFromState();
    sendAppliedImpact(transition.impact);
}

void StartTestExecutionUseCase::sendAppliedImpact(const domain::WindImpact &profile) {
    telemetryClient.setAxisCommand(domain::axis0, domain::axis0WindCommand(profile));
    telemetryClient.setAxisCommand(domain::axis1, domain::axis1WindCommand(profile));
}

} // namespace application::useCases
