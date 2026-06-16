#include "ShellPresenter.hpp"

#include "../Domain/FormulaTemplate.hpp"
#include "../Domain/StandConnectionStatus.hpp"
#include "../Domain/StandConnectionTransitions.hpp"
#include "../Domain/TestExecutionTransitions.hpp"
#include "../Domain/TestModeStatePolicy.hpp"
#include "../Domain/TestProtocol.hpp"
#include "../Domain/TestTimeSource.hpp"
#include "../Domain/TestTimeDirection.hpp"
#include <exception>

namespace presentation {

ShellPresenter::ShellPresenter(Dependencies deps)
    : state(deps.state), startTestExecutionUseCase(deps.startTestExecutionUseCase),
      pauseTestExecutionUseCase(deps.pauseTestExecutionUseCase),
      resumeTestExecutionUseCase(deps.resumeTestExecutionUseCase),
      stopTestExecutionUseCase(deps.stopTestExecutionUseCase),
      setFunctionExpressionUseCase(deps.setFunctionExpressionUseCase), setLineColorUseCase(deps.setLineColorUseCase),
      buildControlPlotUseCase(deps.buildControlPlotUseCase), setTestTimeSourceUseCase(deps.setTestTimeSourceUseCase),
      configureTelemetryUseCase(deps.configureTelemetryUseCase), connectStandUseCase(deps.connectStandUseCase),
      disconnectStandUseCase(deps.disconnectStandUseCase), setStandControlModeUseCase(deps.setStandControlModeUseCase) {
}

void ShellPresenter::attachView(IShellView &view) {
    this->view = &view;
}

void ShellPresenter::detachView() {
    view = nullptr;
}

void ShellPresenter::onViewReady() {
    refreshFromState();
}

void ShellPresenter::onStateChanged() {
    refreshFromState();
}

void ShellPresenter::onStartPressed() {
    startTestExecutionUseCase.execute();
    refreshFromState();

    if (view != nullptr) {
        view->appendLog("Test execution started");
    }
}

void ShellPresenter::onPausePressed() {
    pauseTestExecutionUseCase.execute();
    refreshFromState();

    if (view != nullptr) {
        view->appendLog("Test execution paused");
    }
}

void ShellPresenter::onResumePressed() {
    resumeTestExecutionUseCase.execute();
    refreshFromState();

    if (view != nullptr) {
        view->appendLog("Test execution resumed");
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
        view->appendLog("Test execution stopped");
    }
}

void ShellPresenter::onCalculatePressed() {
    buildControlPlotUseCase.execute();

    if (view != nullptr) {
        view->appendLog("Formula plot rebuilt");
    }
}

void ShellPresenter::onFunctionEdited(std::string expression) {
    setFunctionExpressionUseCase.execute(std::move(expression));

    if (view != nullptr) {
        view->appendLog("Function expression updated");
    }
}

void ShellPresenter::onFormulaTemplateSelected(std::string key) {
    const auto formulaTemplate = domain::formulaTemplateByKey(key);
    setFunctionExpressionUseCase.execute(std::string{formulaTemplate.expression});

    if (view != nullptr) {
        view->appendLog(std::string{"Formula template selected: "} + std::string{formulaTemplate.title});
    }
}

void ShellPresenter::onLineColorSelected(application::dto::RgbColor color) {
    setLineColorUseCase.execute(color);

    if (view != nullptr) {
        view->appendLog("Line color updated");
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
    view->setPauseResumeText(canResume(session.execution.testExecutionStatus) ? "Продолжить" : "Пауза");
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
        view->appendLog("Test time source updated");
    }
}

void ShellPresenter::onStandControlModeChanged(domain::StandControlMode mode) {
    setStandControlModeUseCase.execute(mode);

    if (view != nullptr) {
        view->appendLog("Stand control mode updated");
    }
}

void ShellPresenter::onConnectTelemetryPressed(std::string configPath) {
    try {
        switch (domain::connectionButtonAction(state.get().connection.standConnectionStatus)) {
        case domain::StandConnectionButtonAction::Disconnect:
            disconnectStandUseCase.execute();

            if (view != nullptr) {
                view->appendLog("Stand connection stopped");
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
            view->appendLog("Stand connection started");
        }

        refreshFromState();
    } catch (const std::exception &e) {
        if (view != nullptr) {
            const std::string message = std::string{"Stand connection failed: "} + e.what();
            view->appendLog(message);
            view->showOperatorWarning("Ошибка подключения стенда",
                                      std::string{"Не удалось подключиться к стенду: "} + e.what());
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
        view->setStandConnectionButtonText("Подключить стенд");
        break;
    case domain::StandConnectionButtonAction::Disconnect:
        view->setStandConnectionButtonText("Отключить стенд");
        break;
    }
}

void ShellPresenter::refreshStandConnectionStatusText() {
    if (view == nullptr) {
        return;
    }

    switch (state.get().connection.standConnectionStatus) {
    case domain::StandConnectionStatus::Disconnected:
        view->setStandConnectionStatusText("Стенд: отключен");
        break;
    case domain::StandConnectionStatus::Configured:
        view->setStandConnectionStatusText("Стенд: настроен");
        break;
    case domain::StandConnectionStatus::Connecting:
        view->setStandConnectionStatusText("Стенд: подключение");
        break;
    case domain::StandConnectionStatus::Connected:
        view->setStandConnectionStatusText("Стенд: подключен");
        break;
    case domain::StandConnectionStatus::Polling:
        view->setStandConnectionStatusText("Стенд: обмен активен");
        break;
    case domain::StandConnectionStatus::Disconnecting:
        view->setStandConnectionStatusText("Стенд: отключение");
        break;
    case domain::StandConnectionStatus::Error:
        view->setStandConnectionStatusText("Стенд: ошибка связи");
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

        const std::string message = "Stand connection lost";
        view->appendLog(message);
        view->showOperatorWarning("Связь со стендом потеряна",
                                  "Обмен телеметрией остановлен. Проверьте питание стенда, сеть и состояние mock/real "
                                  "server.");
    }

    lastStandConnectionStatus = status;
}

} // namespace presentation
