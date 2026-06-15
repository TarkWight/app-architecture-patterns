#include "StartTestExecutionUseCase.hpp"

#include "../../Domain/AxisId.hpp"
#include "../../Domain/ScenarioExecutionEngine.hpp"
#include "../../Domain/StandCommandMapper.hpp"
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
      buildControlPlotUseCase(buildControlPlotUseCase) {
}

void StartTestExecutionUseCase::execute() {
    const auto &session = state.get();
    const auto transition = domain::transitionAfterStartRequested(session.testExecutionStatus);
    if (!transition.has_value()) {
        return;
    }

    if (domain::TestModeStatePolicy::usesControlProfile(session.testProtocol.testMode)) {
        buildControlPlotUseCase.execute();
        state.setTargetStandImpact(state.get().windImpact);
        state.clearControlTrace();
    }

    const auto plan = domain::TestExecutionPlanner::plan(session.testProtocol, session.testTimeSource,
                                                         session.estimatedTestDuration, session.operatorTestDuration);

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
            const auto completion = domain::transitionAfterExecutionCompleted(state.get().testExecutionStatus);
            if (completion.has_value()) {
                state.setTestExecutionStatus(*completion);
            }
        }
    });
}

void StartTestExecutionUseCase::startTelemetryPollingIfConnected() {
    const auto transition = domain::transitionAfterPollingStarted(state.get().standConnectionStatus);
    if (!transition.has_value()) {
        return;
    }

    telemetryClient.startPolling(state.get().telemetryPollInterval.milliseconds());
    state.setStandConnectionStatus(*transition);
}

void StartTestExecutionUseCase::stopTelemetryPollingIfActive() {
    const auto transition = domain::transitionAfterPollingStopped(state.get().standConnectionStatus);
    if (!transition.has_value()) {
        return;
    }

    telemetryClient.stopPolling();
    state.setStandConnectionStatus(*transition);
}

void StartTestExecutionUseCase::applyScenarioImpact(domain::ElapsedSeconds elapsed) {
    const auto &session = state.get();
    if (!domain::TestModeStatePolicy::usesControlProfile(session.testProtocol.testMode)) {
        return;
    }

    const auto step =
        domain::ScenarioExecutionEngine::advance(session.controlProfile, elapsed, session.targetStandImpact);
    if (!step.has_value()) {
        return;
    }

    state.setTargetStandImpact(step->impact);
    state.setAppliedStandImpact(step->impact);
    state.appendControlTraceSample(step->traceSample);
    buildControlPlotUseCase.refreshFromState();
    sendAppliedImpact(step->impact);
}

void StartTestExecutionUseCase::sendAppliedImpact(const domain::WindImpact &profile) {
    const auto commands = domain::StandCommandMapper::map(profile);
    telemetryClient.setAxisCommand(domain::axis0, commands.axis0);
    telemetryClient.setAxisCommand(domain::axis1, commands.axis1);
}

} // namespace application::useCases
