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

#include <QtCore/QLoggingCategory>

namespace application::useCases {

namespace {

Q_LOGGING_CATEGORY(logUseCase, "mvp.usecase")

QString standConnectionStatusName(domain::StandConnectionStatus status) {
    switch (status) {
        case domain::StandConnectionStatus::Disconnected:
            return QStringLiteral("Disconnected");
        case domain::StandConnectionStatus::Configured:
            return QStringLiteral("Configured");
        case domain::StandConnectionStatus::Connecting:
            return QStringLiteral("Connecting");
        case domain::StandConnectionStatus::Connected:
            return QStringLiteral("Connected");
        case domain::StandConnectionStatus::Polling:
            return QStringLiteral("Polling");
        case domain::StandConnectionStatus::Disconnecting:
            return QStringLiteral("Disconnecting");
        case domain::StandConnectionStatus::Error:
            return QStringLiteral("Error");
    }

    return QStringLiteral("Unknown");
}

QString testExecutionStatusName(domain::TestExecutionStatus status) {
    switch (status) {
        case domain::TestExecutionStatus::Ready:
            return QStringLiteral("Ready");
        case domain::TestExecutionStatus::Running:
            return QStringLiteral("Running");
        case domain::TestExecutionStatus::Paused:
            return QStringLiteral("Paused");
        case domain::TestExecutionStatus::Completed:
            return QStringLiteral("Completed");
        case domain::TestExecutionStatus::Idle:
            return QStringLiteral("Idle");
        case domain::TestExecutionStatus::Aborted:
            return QStringLiteral("Aborted");
        case domain::TestExecutionStatus::Failed:
            return QStringLiteral("Failed");
    }

    return QStringLiteral("Unknown");
}

QString testModeName(domain::TestMode mode) {
    switch (mode) {
        case domain::TestMode::Manual:
            return QStringLiteral("Manual");
        case domain::TestMode::Hybrid:
            return QStringLiteral("Hybrid");
        case domain::TestMode::Automatic:
            return QStringLiteral("Automatic");
    }

    return QStringLiteral("Unknown");
}

QString testTimeSourceName(domain::TestTimeSource source) {
    switch (source) {
        case domain::TestTimeSource::AutoCalculated:
            return QStringLiteral("AutoCalculated");
        case domain::TestTimeSource::OperatorDefined:
            return QStringLiteral("OperatorDefined");
        case domain::TestTimeSource::FreeRun:
            return QStringLiteral("FreeRun");
    }

    return QStringLiteral("Unknown");
}

QString testTimeDirectionName(domain::TestTimeDirection direction) {
    switch (direction) {
        case domain::TestTimeDirection::CountUp:
            return QStringLiteral("CountUp");
        case domain::TestTimeDirection::CountDown:
            return QStringLiteral("CountDown");
    }

    return QStringLiteral("Unknown");
}

QString impactText(const domain::WindImpact &impact) {
    return QStringLiteral("beaufort=%1, direction=%2, angleOfAttack=%3")
    .arg(impact.beaufort.value(), 0, 'f', 3)
        .arg(impact.direction.degrees(), 0, 'f', 3)
        .arg(impact.angleOfAttack.degrees(), 0, 'f', 3);
}

} // namespace

StartTestExecutionUseCase::StartTestExecutionUseCase(
    application::session::SessionState &state, application::ports::ITestExecutionScheduler &testExecutionScheduler,
    application::ports::ITelemetryClient &telemetryClient, BuildControlPlotUseCase &buildControlPlotUseCase)
    : state(state), testExecutionScheduler(testExecutionScheduler), telemetryClient(telemetryClient),
      buildControlPlotUseCase(buildControlPlotUseCase) {
}

void StartTestExecutionUseCase::execute() {
    const auto &session = state.get();

    qCInfo(logUseCase) << "StartTestExecutionUseCase execute"
                       << "executionStatus=" << testExecutionStatusName(session.testExecutionStatus)
                       << "standStatus=" << standConnectionStatusName(session.standConnectionStatus)
                       << "testMode=" << testModeName(session.testProtocol.testMode)
                       << "timeSource=" << testTimeSourceName(session.testTimeSource)
                       << "estimatedMinutes=" << session.estimatedTestDuration.value()
                       << "operatorMinutes=" << session.operatorTestDuration.value()
                       << "telemetryPollIntervalMs=" << session.telemetryPollIntervalMs;

    if (!domain::canStart(session.testExecutionStatus)) {
        qCInfo(logUseCase) << "StartTestExecutionUseCase skipped"
                           << "reason=cannot_start"
                           << "executionStatus=" << testExecutionStatusName(session.testExecutionStatus);
        return;
    }

    if (session.testProtocol.testMode != domain::TestMode::Manual) {
        qCInfo(logUseCase) << "StartTestExecutionUseCase build control plot";
        buildControlPlotUseCase.execute();

        qCInfo(logUseCase) << "StartTestExecutionUseCase reset target impact"
                           << impactText(state.get().windImpact);

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

    qCInfo(logUseCase) << "StartTestExecutionUseCase timing"
                       << "activeDurationMinutes=" << activeDurationMinutes
                       << "direction=" << testTimeDirectionName(direction);

    state.setActiveTestDurationMinutes(activeDurationMinutes);
    state.setTestTimeDirection(direction);
    state.setElapsedSeconds(0);

    if (direction == domain::TestTimeDirection::CountDown) {
        state.setRemainingSeconds(activeDurationMinutes * 60);
    } else {
        state.setRemainingSeconds(0);
    }

    qCInfo(logUseCase) << "StartTestExecutionUseCase status transition"
                       << testExecutionStatusName(session.testExecutionStatus)
                       << "->"
                       << testExecutionStatusName(domain::TestExecutionStatus::Running);

    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);

    startTelemetryPollingIfConnected();
    applyScenarioImpact(0);

    qCInfo(logUseCase) << "StartTestExecutionUseCase scheduler start";

    testExecutionScheduler.start(0, [this](int elapsedSeconds) {
        qCDebug(logUseCase) << "scheduler tick"
                            << "elapsedSeconds=" << elapsedSeconds;

        state.setElapsedSeconds(elapsedSeconds);
        applyScenarioImpact(elapsedSeconds);

        const auto &current = state.get();

        if (current.testTimeDirection == domain::TestTimeDirection::CountDown) {
            const int totalSeconds = current.activeTestDuration.value() * 60;
            const int remainingSeconds = (elapsedSeconds < totalSeconds) ? (totalSeconds - elapsedSeconds) : 0;

            qCDebug(logUseCase) << "countdown update"
                                << "elapsedSeconds=" << elapsedSeconds
                                << "totalSeconds=" << totalSeconds
                                << "remainingSeconds=" << remainingSeconds;

            state.setRemainingSeconds(remainingSeconds);

            if (remainingSeconds == 0) {
                qCInfo(logUseCase) << "countdown completed";

                testExecutionScheduler.stop();
                stopTelemetryPollingIfActive();

                qCInfo(logUseCase) << "test status transition"
                                   << testExecutionStatusName(current.testExecutionStatus)
                                   << "->"
                                   << testExecutionStatusName(domain::TestExecutionStatus::Completed);

                state.setTestExecutionStatus(domain::TestExecutionStatus::Completed);
            }
        }
    });
}

void StartTestExecutionUseCase::startTelemetryPollingIfConnected() {
    const auto currentStatus = state.get().standConnectionStatus;

    qCInfo(logUseCase) << "startTelemetryPollingIfConnected"
                       << "standStatus=" << standConnectionStatusName(currentStatus);

    if (currentStatus != domain::StandConnectionStatus::Connected) {
        qCInfo(logUseCase) << "startTelemetryPollingIfConnected skipped"
                           << "reason=stand_not_connected";
        return;
    }

    const int intervalMs = state.get().telemetryPollIntervalMs;

    qCInfo(logUseCase) << "telemetryClient.startPolling"
                       << "intervalMs=" << intervalMs;

    telemetryClient.startPolling(intervalMs);

    qCInfo(logUseCase) << "stand status transition"
                       << standConnectionStatusName(domain::StandConnectionStatus::Connected)
                       << "->"
                       << standConnectionStatusName(domain::StandConnectionStatus::Polling);

    state.setStandConnectionStatus(domain::StandConnectionStatus::Polling);
}

void StartTestExecutionUseCase::stopTelemetryPollingIfActive() {
    const auto currentStatus = state.get().standConnectionStatus;

    qCInfo(logUseCase) << "stopTelemetryPollingIfActive"
                       << "standStatus=" << standConnectionStatusName(currentStatus);

    if (currentStatus != domain::StandConnectionStatus::Polling) {
        qCInfo(logUseCase) << "stopTelemetryPollingIfActive skipped"
                           << "reason=stand_not_polling";
        return;
    }

    qCInfo(logUseCase) << "telemetryClient.stopPolling";
    telemetryClient.stopPolling();

    qCInfo(logUseCase) << "stand status transition"
                       << standConnectionStatusName(domain::StandConnectionStatus::Polling)
                       << "->"
                       << standConnectionStatusName(domain::StandConnectionStatus::Connected);

    state.setStandConnectionStatus(domain::StandConnectionStatus::Connected);
}

void StartTestExecutionUseCase::applyScenarioImpact(int elapsedSeconds) {
    const auto &session = state.get();

    qCDebug(logUseCase) << "applyScenarioImpact"
                        << "elapsedSeconds=" << elapsedSeconds
                        << "testMode=" << testModeName(session.testProtocol.testMode);

    if (session.testProtocol.testMode == domain::TestMode::Manual) {
        qCDebug(logUseCase) << "applyScenarioImpact skipped"
                            << "reason=manual_mode";
        return;
    }

    const auto impact = domain::windImpactAt(session.controlProfile, domain::ElapsedSeconds::from(elapsedSeconds),
                                             session.targetStandImpact);
    if (!impact.has_value()) {
        qCWarning(logUseCase) << "applyScenarioImpact skipped"
                              << "reason=no_impact_for_time"
                              << "elapsedSeconds=" << elapsedSeconds;
        return;
    }

    qCInfo(logUseCase) << "applyScenarioImpact target"
                       << "elapsedSeconds=" << elapsedSeconds
                       << impactText(*impact);

           // TODO:
           // const auto transition = domain::StandImpactTransition{}.advance(session.appliedStandImpact, *impact);

           // state.setTargetStandImpact(*impact);
           // state.setAppliedStandImpact(transition.impact);
           // state.appendControlTraceSample(domain::ControlTraceSample{.timeSeconds = static_cast<double>(elapsedSeconds),
           //                                                           .targetValue = *impact,
           //                                                           .safeCommandValue = transition.impact});
           // buildControlPlotUseCase.refreshFromState();
           // sendAppliedImpact(transition.impact);

    state.setTargetStandImpact(*impact);
    state.setAppliedStandImpact(*impact);
    state.appendControlTraceSample(domain::ControlTraceSample{
                                                              .timeSeconds = static_cast<double>(elapsedSeconds), .targetValue = *impact, .safeCommandValue = *impact});
    buildControlPlotUseCase.refreshFromState();

    sendAppliedImpact(*impact);
}

void StartTestExecutionUseCase::sendAppliedImpact(const domain::WindImpact &profile) {
    qCInfo(logUseCase) << "sendAppliedImpact"
                       << impactText(profile);

    const auto axis0Command = domain::axis0WindCommand(profile);
    const auto axis1Command = domain::axis1WindCommand(profile);

    qCInfo(logUseCase) << "axis0 command"
                       << "position=" << axis0Command.position
                       << "velocity=" << axis0Command.velocity
                       << "torque=" << axis0Command.torque
                       << "flags=" << axis0Command.cmd1 << axis0Command.cmd2 << axis0Command.cmd3 << axis0Command.cmd4;

    qCInfo(logUseCase) << "axis1 command"
                       << "position=" << axis1Command.position
                       << "velocity=" << axis1Command.velocity
                       << "torque=" << axis1Command.torque
                       << "flags=" << axis1Command.cmd1 << axis1Command.cmd2 << axis1Command.cmd3 << axis1Command.cmd4;

    telemetryClient.setAxisCommand(domain::axis0, axis0Command);
    telemetryClient.setAxisCommand(domain::axis1, axis1Command);
}

} // namespace application::useCases