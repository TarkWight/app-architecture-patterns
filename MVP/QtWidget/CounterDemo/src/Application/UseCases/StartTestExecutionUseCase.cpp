#include "StartTestExecutionUseCase.hpp"

#include "../../Domain/ScenarioExecutionEngine.hpp"
#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/StandConnectionTransitions.hpp"
#include "../../Domain/TestExecutionPlanner.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"
#include "../../Domain/TestModeStatePolicy.hpp"
#include "../../Domain/TestProtocol.hpp"

namespace application::useCases {

StartTestExecutionUseCase::StartTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler,
    application::ports::ITelemetryClient &telemetryClient, BuildControlPlotUseCase &buildControlPlotUseCase)
    : state(state), testExecutionScheduler(testExecutionScheduler), telemetryClient(telemetryClient),
      buildControlPlotUseCase(buildControlPlotUseCase), estimateTestDurationUseCase(state),
      appliedStandImpactSender(telemetryClient) {
}

void StartTestExecutionUseCase::execute() {
    const auto transition = domain::transitionAfterStartRequested(state.execution().testExecutionStatus);
    if (!transition.has_value()) {
        return;
    }

    estimateTestDurationUseCase.executeForAutoCalculated();

    if (domain::TestModeStatePolicy::usesControlProfile(state.protocol().testProtocol.testMode)) {
        buildControlPlotUseCase.execute();
        state.setTargetStandImpact(state.control().windImpact);
        state.clearControlTrace();
    }

    const auto &protocol = state.protocol();
    const auto plan = domain::TestExecutionPlanner::plan(protocol.testProtocol, protocol.testTimeSource,
                                                         protocol.estimatedTestDuration, protocol.operatorTestDuration);

    state.setActiveTestDurationMinutes(plan.activeDuration);
    state.setTestTimeDirection(plan.direction);
    state.setElapsedSeconds(domain::ElapsedSeconds::from(0));
    state.setRemainingSeconds(plan.initialRemaining());

    state.setTestExecutionStatus(*transition);
    startTelemetryPollingIfConnected();
    applyScenarioImpact(domain::ElapsedSeconds::from(0));

    testExecutionScheduler.start(0, [this, plan](int elapsedSeconds) {
        const auto elapsed = domain::ElapsedSeconds::from(elapsedSeconds);
        state.setElapsedSeconds(elapsed);
        applyScenarioImpact(elapsed);

        const auto remaining = plan.remainingAt(elapsed);

        state.setRemainingSeconds(remaining);

        if (plan.isCompletedAt(elapsed)) {
            testExecutionScheduler.stop();
            stopTelemetryPollingIfActive();
            const auto completion = domain::transitionAfterExecutionCompleted(state.execution().testExecutionStatus);
            if (completion.has_value()) {
                state.setTestExecutionStatus(*completion);
            }
        }
    });
}

void StartTestExecutionUseCase::startTelemetryPollingIfConnected() {
    const auto transition = domain::transitionAfterPollingStarted(state.connection().standConnectionStatus);
    if (!transition.has_value()) {
        return;
    }

    telemetryClient.startPolling(state.connection().telemetryPollInterval.milliseconds());
    state.setStandConnectionStatus(*transition);
}

void StartTestExecutionUseCase::stopTelemetryPollingIfActive() {
    const auto transition = domain::transitionAfterPollingStopped(state.connection().standConnectionStatus);
    if (!transition.has_value()) {
        return;
    }

    telemetryClient.stopPolling();
    state.setStandConnectionStatus(*transition);
}

void StartTestExecutionUseCase::applyScenarioImpact(domain::ElapsedSeconds elapsed) {
    const auto &protocol = state.protocol();
    if (!domain::TestModeStatePolicy::usesControlProfile(protocol.testProtocol.testMode)) {
        return;
    }

    const auto &control = state.control();
    const auto step =
        domain::ScenarioExecutionEngine::advance(control.controlProfile, elapsed, control.targetStandImpact);
    if (!step.has_value()) {
        return;
    }

    state.setTargetStandImpact(step->impact);
    state.setAppliedStandImpact(step->impact);
    state.appendControlTraceSample(step->traceSample);
    buildControlPlotUseCase.refreshFromState();
    appliedStandImpactSender.send(step->impact, elapsed, protocol.testProtocol);
}

} // namespace application::useCases
