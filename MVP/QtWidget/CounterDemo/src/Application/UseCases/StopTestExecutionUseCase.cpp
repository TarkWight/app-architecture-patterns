#include "StopTestExecutionUseCase.hpp"

#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"
#include "../../Domain/TestTimeDirection.hpp"

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

QString testTimeDirectionName(domain::TestTimeDirection direction) {
    switch (direction) {
        case domain::TestTimeDirection::CountUp:
            return QStringLiteral("CountUp");
        case domain::TestTimeDirection::CountDown:
            return QStringLiteral("CountDown");
    }

    return QStringLiteral("Unknown");
}

} // namespace

StopTestExecutionUseCase::StopTestExecutionUseCase(application::session::SessionState &state,
                                                   application::ports::ITestExecutionScheduler &testExecutionScheduler,
                                                   application::ports::ITelemetryClient &telemetryClient)
    : state(state), testExecutionScheduler(testExecutionScheduler), telemetryClient(telemetryClient) {
}

void StopTestExecutionUseCase::execute() {
    const auto &initialSession = state.get();

    qCInfo(logUseCase) << "StopTestExecutionUseCase execute"
                       << "executionStatus=" << testExecutionStatusName(initialSession.testExecutionStatus)
                       << "standStatus=" << standConnectionStatusName(initialSession.standConnectionStatus)
                       << "timeDirection=" << testTimeDirectionName(initialSession.testTimeDirection)
                       << "elapsedSeconds=" << initialSession.elapsed.value()
                       << "remainingSeconds=" << initialSession.remaining.value()
                       << "activeDurationMinutes=" << initialSession.activeTestDuration.value();

    if (!domain::canStop(initialSession.testExecutionStatus)) {
        qCInfo(logUseCase) << "StopTestExecutionUseCase skipped"
                           << "reason=cannot_stop"
                           << "executionStatus=" << testExecutionStatusName(initialSession.testExecutionStatus);
        return;
    }

    qCInfo(logUseCase) << "testExecutionScheduler.stop";
    testExecutionScheduler.stop();

    if (state.get().standConnectionStatus == domain::StandConnectionStatus::Polling) {
        qCInfo(logUseCase) << "telemetryClient.stopPolling";
        telemetryClient.stopPolling();

        qCInfo(logUseCase) << "stand status transition"
                           << standConnectionStatusName(domain::StandConnectionStatus::Polling)
                           << "->"
                           << standConnectionStatusName(domain::StandConnectionStatus::Connected);

        state.setStandConnectionStatus(domain::StandConnectionStatus::Connected);
    } else {
        qCInfo(logUseCase) << "telemetry stop skipped"
                           << "reason=stand_not_polling"
                           << "standStatus=" << standConnectionStatusName(state.get().standConnectionStatus);
    }

    const auto &session = state.get();

    qCInfo(logUseCase) << "reset test time"
                       << "direction=" << testTimeDirectionName(session.testTimeDirection);

    state.setElapsedSeconds(0);

    if (session.testTimeDirection == domain::TestTimeDirection::CountDown) {
        const int restoredRemainingSeconds = session.activeTestDuration.value() * 60;

        qCInfo(logUseCase) << "restore countdown"
                           << "remainingSeconds=" << restoredRemainingSeconds;

        state.setRemainingSeconds(restoredRemainingSeconds);
    } else {
        qCInfo(logUseCase) << "reset countup remaining=0";
        state.setRemainingSeconds(0);
    }

    qCInfo(logUseCase) << "test status transition"
                       << testExecutionStatusName(session.testExecutionStatus)
                       << "->"
                       << testExecutionStatusName(domain::TestExecutionStatus::Ready);

    state.setTestExecutionStatus(domain::TestExecutionStatus::Ready);
}

} // namespace application::useCases