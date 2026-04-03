#include "ShellPresenter.hpp"

namespace presentation {

ShellPresenter::ShellPresenter(Dependencies deps)
    : state(deps.state), startTimerUseCase(deps.startTimerUseCase), stopTimerUseCase(deps.stopTimerUseCase),
      setFunctionExpressionUseCase(deps.setFunctionExpressionUseCase), setLineColorUseCase(deps.setLineColorUseCase),
      buildControlPlotUseCase(deps.buildControlPlotUseCase) {
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

void ShellPresenter::onStartPressed() {
    startTimerUseCase.execute();
    refreshFromState();

    if (view != nullptr) {
        view->appendLog("Timer started");
    }
}

void ShellPresenter::onStopPressed() {
    stopTimerUseCase.execute();
    refreshFromState();

    if (view != nullptr) {
        view->appendLog("Timer stopped");
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

std::string ShellPresenter::formatTimerText(int elapsedSeconds) {
    const int minutes = elapsedSeconds / 60;
    const int seconds = elapsedSeconds % 60;

    std::string secondsText = (seconds < 10) ? "0" + std::to_string(seconds) : std::to_string(seconds);

    return std::to_string(minutes) + ":" + secondsText;
}

void ShellPresenter::refreshFromState() {
    if (view == nullptr) {
        return;
    }

    const auto &session = state.get();

    view->setTimerText(formatTimerText(session.elapsed.value));
    view->setStartEnabled(!session.timerRunning);
    view->setStopEnabled(session.timerRunning);
    view->setFunctionExpression(session.functionExpression.value);
}

} // namespace presentation
