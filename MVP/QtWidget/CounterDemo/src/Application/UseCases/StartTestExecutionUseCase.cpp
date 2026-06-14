#include "StartTestExecutionUseCase.hpp"

#include "../../Domain/AxisId.hpp"
#include "../../Domain/StandCommandMapper.hpp"
#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/TestExecutionPlanner.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"
#include "../../Domain/TestProtocol.hpp"
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

    const auto plan = domain::TestExecutionPlanner::plan(session.testProtocol, session.testTimeSource,
                                                         session.estimatedTestDuration, session.operatorTestDuration);

    state.setActiveTestDurationMinutes(plan.activeDuration.value());
    state.setTestTimeDirection(plan.direction);
    state.setElapsedSeconds(0);
    state.setRemainingSeconds(plan.initialRemaining().value());

    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
    startTelemetryPollingIfConnected();
    applyScenarioImpact(0);

    testExecutionScheduler.start(0, [this, plan](int elapsedSeconds) {
        state.setElapsedSeconds(elapsedSeconds);
        applyScenarioImpact(elapsedSeconds);

        const auto remaining = plan.remainingAt(domain::ElapsedSeconds::from(elapsedSeconds));

        state.setRemainingSeconds(remaining.value());

        if (plan.isCompletedAt(domain::ElapsedSeconds::from(elapsedSeconds))) {
            testExecutionScheduler.stop();
            stopTelemetryPollingIfActive();
            state.setTestExecutionStatus(domain::TestExecutionStatus::Completed);
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

    state.setTargetStandImpact(*impact);
    state.setAppliedStandImpact(*impact);
    state.appendControlTraceSample(domain::ControlTraceSample{
        .timeSeconds = static_cast<double>(elapsedSeconds), .targetValue = *impact, .safeCommandValue = *impact});
    buildControlPlotUseCase.refreshFromState();
    sendAppliedImpact(*impact);
}

void StartTestExecutionUseCase::sendAppliedImpact(const domain::WindImpact &profile) {
    const auto commands = domain::StandCommandMapper::map(profile);
    telemetryClient.setAxisCommand(domain::axis0, commands.axis0);
    telemetryClient.setAxisCommand(domain::axis1, commands.axis1);
}

} // namespace application::useCases
