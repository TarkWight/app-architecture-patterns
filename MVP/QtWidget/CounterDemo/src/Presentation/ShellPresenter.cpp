#include "ShellPresenter.hpp"

#include "../Domain/StandConnectionStatus.hpp"
#include "../Domain/StandConnectionTransitions.hpp"
#include "../Domain/TestExecutionTransitions.hpp"
#include "../Domain/TestModeStatePolicy.hpp"
#include "../Domain/TestProtocol.hpp"
#include "../Domain/TestTimeSource.hpp"
#include "../Domain/TestTimeDirection.hpp"
#include "../Localization/ShellStrings.hpp"
#include <exception>

namespace presentation {

ShellPresenter::ShellPresenter(Dependencies deps)
    : state(deps.state), startTestExecutionUseCase(deps.startTestExecutionUseCase),
      pauseTestExecutionUseCase(deps.pauseTestExecutionUseCase),
      resumeTestExecutionUseCase(deps.resumeTestExecutionUseCase),
      stopTestExecutionUseCase(deps.stopTestExecutionUseCase),
      setFunctionExpressionUseCase(deps.setFunctionExpressionUseCase), setLineColorUseCase(deps.setLineColorUseCase),
      calculateAndBuildControlPlotUseCase(deps.calculateAndBuildControlPlotUseCase),
      estimateTestDurationUseCase(deps.estimateTestDurationUseCase),
      setTestTimeSourceUseCase(deps.setTestTimeSourceUseCase),
      configureTelemetryUseCase(deps.configureTelemetryUseCase), connectStandUseCase(deps.connectStandUseCase),
      disconnectStandUseCase(deps.disconnectStandUseCase), setStandControlModeUseCase(deps.setStandControlModeUseCase),
      telemetryClient(deps.telemetryClient) {
}

void ShellPresenter::attachView(IShellView &view) {
    this->view = &view;
    telemetryClient.setTraceCallback([this](const std::string &message) {
        if (this->view != nullptr) {
            this->view->appendLog(message);
        }
    });
}

void ShellPresenter::detachView() {
    telemetryClient.setTraceCallback({});
    view = nullptr;
}

void ShellPresenter::onViewReady() {
    refreshFromState();
}

void ShellPresenter::onStateChanged() {
    refreshFromState();
}

void ShellPresenter::onStartPressed() {
    if (!connectionAllowsStart()) {
        return;
    }

    if (!readinessAllowsStart()) {
        refreshFromState();
        return;
    }

    startTestExecutionUseCase.execute();
    refreshFromState();

    if (view != nullptr) {
        view->appendLog(localization::shell::testExecutionStarted);
    }
}

bool ShellPresenter::readinessAllowsStart() {
    const auto mode = state.protocol().testProtocol.testMode;
    if (!readinessGateRequired(mode)) {
        return true;
    }

    if (state.readiness().status == application::session::ReadinessStatus::Unknown) {
        estimateTestDurationUseCase.executeForAutoCalculated();
    }

    const auto status = state.readiness().status;
    if (!readinessConfirmationRequired(status)) {
        return true;
    }

    return confirmDangerousReadinessStart(status);
}

bool ShellPresenter::connectionAllowsStart() {
    if (state.connection().standConnectionStatus == domain::StandConnectionStatus::Connected) {
        return true;
    }

    if (view != nullptr) {
        view->showOperatorWarning(localization::shell::standNotConnectedTitle,
                                  localization::shell::standNotConnectedMessage);
        view->appendLog(localization::shell::startBlockedStandDisconnected);
    }

    return false;
}

bool ShellPresenter::confirmDangerousReadinessStart(application::session::ReadinessStatus status) {
    if (view == nullptr) {
        return false;
    }

    const std::string details = status == application::session::ReadinessStatus::Failed
                                    ? localization::shell::readinessFailedStartDetails
                                    : localization::shell::readinessDangerousStartDetails;

    return view->confirmDangerousReadinessStart(localization::shell::dangerousStartConfirmationTitle,
                                                details + localization::shell::continueStartQuestion);
}

void ShellPresenter::onPausePressed() {
    pauseTestExecutionUseCase.execute();
    refreshFromState();

    if (view != nullptr) {
        view->freezeStandImpactTransition();
        view->appendLog(localization::shell::testExecutionPaused);
    }
}

void ShellPresenter::onResumePressed() {
    resumeTestExecutionUseCase.execute();
    refreshFromState();

    if (view != nullptr) {
        view->appendLog(localization::shell::testExecutionResumed);
    }
}

void ShellPresenter::onPauseResumePressed() {
    const auto status = state.get().execution.testExecutionStatus;
    if (canPause(status)) {
        onPausePressed();
        return;
    }

    if (canResume(status)) {
        onResumePressed();
    }
}

void ShellPresenter::onStopPressed() {
    stopTestExecutionUseCase.execute();
    refreshFromState();

    if (view != nullptr) {
        view->freezeStandImpactTransition();
        view->appendLog(localization::shell::testExecutionStopped);
    }
}

void ShellPresenter::onCalculatePressed() {
    calculateAndBuildControlPlotUseCase.execute();

    if (view != nullptr) {
        view->appendLog(localization::shell::readinessAndPlotBuilt);
    }
}

void ShellPresenter::onFunctionEdited(std::string expression) {
    setFunctionExpressionUseCase.execute(std::move(expression));

    if (view != nullptr) {
        view->appendLog(localization::shell::functionExpressionUpdated);
    }
}

void ShellPresenter::onLineColorSelected(application::dto::RgbColor color) {
    setLineColorUseCase.execute(color);

    if (view != nullptr) {
        view->appendLog(localization::shell::lineColorUpdated);
    }
}

std::string ShellPresenter::formatTimerText(int secondsValue) {
    const int minutes = secondsValue / 60;
    const int seconds = secondsValue % 60;

    const std::string secondsText = (seconds < 10) ? "0" + std::to_string(seconds) : std::to_string(seconds);

    return std::to_string(minutes) + ":" + secondsText;
}

bool ShellPresenter::canStart(domain::TestExecutionStatus status) {
    return domain::canStart(status);
}

bool ShellPresenter::canPause(domain::TestExecutionStatus status) {
    return domain::canPause(status);
}

bool ShellPresenter::canResume(domain::TestExecutionStatus status) {
    return domain::canResume(status);
}

bool testTimeSourceCanBeChanged(domain::TestMode mode) {
    return domain::TestModeStatePolicy::allowsOperatorDuration(mode);
}

bool ShellPresenter::canStop(domain::TestExecutionStatus status) {
    return domain::canStop(status);
}

bool ShellPresenter::readinessGateRequired(domain::TestMode mode) {
    return mode == domain::TestMode::Hybrid || mode == domain::TestMode::Automatic;
}

bool ShellPresenter::readinessConfirmationRequired(application::session::ReadinessStatus status) {
    return status == application::session::ReadinessStatus::Dangerous ||
           status == application::session::ReadinessStatus::Failed;
}

void ShellPresenter::refreshFromState() {
    if (view == nullptr) {
        return;
    }

    const auto &session = state.get();

    const int displayedSeconds = (session.execution.testTimeDirection == domain::TestTimeDirection::CountDown)
                                     ? session.execution.remaining.value()
                                     : session.execution.elapsed.value();

    view->setTimerText(formatTimerText(displayedSeconds));
    view->setStartEnabled(canStart(session.execution.testExecutionStatus));
    view->setPauseResumeEnabled(canPause(session.execution.testExecutionStatus) ||
                                canResume(session.execution.testExecutionStatus));
    view->setPauseResumeText(canResume(session.execution.testExecutionStatus) ? localization::shell::resumeButton
                                                                              : localization::shell::pauseButton);
    view->setStopEnabled(canStop(session.execution.testExecutionStatus));
    view->setFunctionExpression(session.control.functionExpression.value);
    view->setTestTimeSource(session.protocol.testTimeSource);
    view->setTestTimeSourceEnabled(testTimeSourceCanBeChanged(session.protocol.testProtocol.testMode));
    refreshStandConnectionButton();
    refreshStandConnectionStatusText();
    notifyStandConnectionStatusChanged(session.connection.standConnectionStatus);
}

void ShellPresenter::onTestTimeSourceChanged(domain::TestTimeSource source) {
    setTestTimeSourceUseCase.execute(source);
    refreshFromState();

    if (view != nullptr) {
        view->appendLog(localization::shell::testTimeSourceUpdated);
    }
}

void ShellPresenter::onStandControlModeChanged(domain::StandControlMode mode) {
    setStandControlModeUseCase.execute(mode);

    if (view != nullptr) {
        view->appendLog(localization::shell::standControlModeUpdated);
    }
}

void ShellPresenter::onConnectTelemetryPressed(std::string configPath) {
    try {
        switch (domain::connectionButtonAction(state.get().connection.standConnectionStatus)) {
        case domain::StandConnectionButtonAction::Disconnect:
            disconnectStandUseCase.execute();

            if (view != nullptr) {
                view->appendLog(localization::shell::standConnectionStopped);
            }

            refreshFromState();
            return;

        case domain::StandConnectionButtonAction::ConfigureAndConnect:
            configureTelemetryUseCase.execute(configPath);
            [[fallthrough]];
        case domain::StandConnectionButtonAction::Connect:
            connectStandUseCase.execute();
            break;
        }

        if (view != nullptr) {
            view->appendLog(localization::shell::standConnectionStarted);
        }

        refreshFromState();
    } catch (const std::exception &e) {
        if (view != nullptr) {
            const std::string message = std::string{localization::shell::standConnectionFailedPrefix} + e.what();
            view->appendLog(message);
            view->showOperatorWarning(localization::shell::standConnectionErrorTitle, message);
        }
    }
}

bool ShellPresenter::telemetryConfigRequiredForConnection() const {
    return domain::connectionButtonAction(state.get().connection.standConnectionStatus) ==
           domain::StandConnectionButtonAction::ConfigureAndConnect;
}

void ShellPresenter::refreshStandConnectionButton() {
    if (view == nullptr) {
        return;
    }

    switch (domain::connectionButtonAction(state.get().connection.standConnectionStatus)) {
    case domain::StandConnectionButtonAction::ConfigureAndConnect:
    case domain::StandConnectionButtonAction::Connect:
        view->setStandConnectionButtonText(localization::shell::connectStandButton);
        break;
    case domain::StandConnectionButtonAction::Disconnect:
        view->setStandConnectionButtonText(localization::shell::disconnectStandButton);
        break;
    }
}

void ShellPresenter::refreshStandConnectionStatusText() {
    if (view == nullptr) {
        return;
    }

    switch (state.get().connection.standConnectionStatus) {
    case domain::StandConnectionStatus::Disconnected:
        view->setStandConnectionStatusText(localization::shell::standDisconnectedStatus);
        break;
    case domain::StandConnectionStatus::Configured:
        view->setStandConnectionStatusText(localization::shell::standConfiguredStatus);
        break;
    case domain::StandConnectionStatus::Connecting:
        view->setStandConnectionStatusText(localization::shell::standConnectingStatus);
        break;
    case domain::StandConnectionStatus::Connected:
        view->setStandConnectionStatusText(localization::shell::standConnectedStatus);
        break;
    case domain::StandConnectionStatus::Polling:
        view->setStandConnectionStatusText(localization::shell::standPollingStatus);
        break;
    case domain::StandConnectionStatus::Disconnecting:
        view->setStandConnectionStatusText(localization::shell::standDisconnectingStatus);
        break;
    case domain::StandConnectionStatus::Error:
        view->setStandConnectionStatusText(localization::shell::standErrorStatus);
        break;
    }
}

void ShellPresenter::notifyStandConnectionStatusChanged(domain::StandConnectionStatus status) {
    if (status == lastStandConnectionStatus) {
        return;
    }

    if (status == domain::StandConnectionStatus::Connected || status == domain::StandConnectionStatus::Polling ||
        status == domain::StandConnectionStatus::Disconnected) {
        standConnectionWarningShown = false;
    }

    if (status == domain::StandConnectionStatus::Error && view != nullptr && !standConnectionWarningShown) {
        standConnectionWarningShown = true;
        lastStandConnectionStatus = status;

        const std::string message = localization::shell::standConnectionLost;
        view->appendLog(message);
        view->showOperatorWarning(localization::shell::standConnectionLostTitle,
                                  localization::shell::standConnectionLostMessage);
    }

    lastStandConnectionStatus = status;
}

} // namespace presentation
