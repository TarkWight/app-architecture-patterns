#include "ShellPresenter.hpp"

#include "../Domain/TestTimeSource.hpp"
#include "../Domain/TestTimeDirection.hpp"

namespace presentation {

ShellPresenter::ShellPresenter(Dependencies deps)
    : state(deps.state),
      startTestExecutionUseCase(deps.startTestExecutionUseCase),
      pauseTestExecutionUseCase(deps.pauseTestExecutionUseCase),
      resumeTestExecutionUseCase(deps.resumeTestExecutionUseCase),
      stopTestExecutionUseCase(deps.stopTestExecutionUseCase),
      setFunctionExpressionUseCase(deps.setFunctionExpressionUseCase),
      setLineColorUseCase(deps.setLineColorUseCase),
      buildControlPlotUseCase(deps.buildControlPlotUseCase),
      setTestTimeSourceUseCase(deps.setTestTimeSourceUseCase) {
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

void ShellPresenter::onLineColorSelected(domain::RgbColor color) {
    setLineColorUseCase.execute(color);

    if (view != nullptr) {
        view->appendLog("Line color updated");
    }
}

std::string ShellPresenter::formatTimerText(int secondsValue) {
    const int minutes = secondsValue / 60;
    const int seconds = secondsValue % 60;

    const std::string secondsText = (seconds < 10)
        ? "0" + std::to_string(seconds)
        : std::to_string(seconds);

    return std::to_string(minutes) + ":" + secondsText;
}

bool ShellPresenter::canStart(domain::TestExecutionStatus status) {
    switch (status) {
        case domain::TestExecutionStatus::Idle:
        case domain::TestExecutionStatus::Ready:
        case domain::TestExecutionStatus::Completed:
        case domain::TestExecutionStatus::Aborted:
        case domain::TestExecutionStatus::Failed:
            return true;
        case domain::TestExecutionStatus::Running:
        case domain::TestExecutionStatus::Paused:
            return false;
    }
    return false;
}

bool ShellPresenter::canPause(domain::TestExecutionStatus status) {
    switch (status) {
    case domain::TestExecutionStatus::Running:
        return true;

    case domain::TestExecutionStatus::Idle:
    case domain::TestExecutionStatus::Ready:
    case domain::TestExecutionStatus::Paused:
    case domain::TestExecutionStatus::Completed:
    case domain::TestExecutionStatus::Aborted:
    case domain::TestExecutionStatus::Failed:
        return false;
    }

    return false;
}

bool ShellPresenter::canResume(domain::TestExecutionStatus status) {
    switch (status) {
    case domain::TestExecutionStatus::Paused:
        return true;

    case domain::TestExecutionStatus::Idle:
    case domain::TestExecutionStatus::Ready:
    case domain::TestExecutionStatus::Running:
    case domain::TestExecutionStatus::Completed:
    case domain::TestExecutionStatus::Aborted:
    case domain::TestExecutionStatus::Failed:
        return false;
    }

    return false;
}

bool ShellPresenter::canStop(domain::TestExecutionStatus status) {
    switch (status) {
        case domain::TestExecutionStatus::Running:
        case domain::TestExecutionStatus::Paused:
            return true;
        case domain::TestExecutionStatus::Idle:
        case domain::TestExecutionStatus::Ready:
        case domain::TestExecutionStatus::Completed:
        case domain::TestExecutionStatus::Aborted:
        case domain::TestExecutionStatus::Failed:
            return false;
    }
    return false;
}

void ShellPresenter::refreshFromState() {
    if (view == nullptr) {
        return;
    }

    const auto &session = state.get();

    const int displayedSeconds =
        (session.testTimeDirection == domain::TestTimeDirection::CountDown)
            ? session.remaining.value
            : session.elapsed.value;

    view->setTimerText(formatTimerText(displayedSeconds));
    view->setStartEnabled(canStart(session.testExecutionStatus));
    view->setPauseEnabled(canPause(session.testExecutionStatus));
    view->setResumeEnabled(canResume(session.testExecutionStatus));
    view->setStopEnabled(canStop(session.testExecutionStatus));
    view->setFunctionExpression(session.functionExpression.value);
    view->setTestTimeSource(session.testTimeSource);
}

void ShellPresenter::onTestTimeSourceChanged(domain::TestTimeSource source) {
    setTestTimeSourceUseCase.execute(source);
    refreshFromState();

    if (view != nullptr) {
        view->appendLog("Test time source updated");
    }
}

} // namespace presentation