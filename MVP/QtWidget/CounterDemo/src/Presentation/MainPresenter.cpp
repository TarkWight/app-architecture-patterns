#include "MainPresenter.hpp"
#include <format>

namespace presentation {

MainPresenter::MainPresenter(useCases::ExecuteCounterCommandUseCase &executeCommandUseCase)
    : executor(executeCommandUseCase) {
}

void MainPresenter::attachView(IMainView &view) {
    this->view = &view;
}

void MainPresenter::detachView() {
    view = nullptr;
}

void MainPresenter::onViewReady() {
    if (!view) {
        return;
    }

    for (int i = 0; i < 3; ++i) {
        view->setCounterValue(domain::CounterId{static_cast<domain::CounterId::Rep>(i)}, 0);
    }
}

void MainPresenter::onTabChanged(int tabIndex) {
    currentCounterId = domain::CounterId{static_cast<domain::CounterId::Rep>(tabIndex)};
    if (view) {
        view->appendCommandLog(std::format("UI: активная вкладка = {}", tabIndex));
    }
}

int MainPresenter::stepFor(domain::CounterId counterId) const {
    return static_cast<int>(counterId.getValue()) + 1;
}

void MainPresenter::executeAndRefresh(const domain::CounterCommand &command, const std::string &log) {
    const auto result = executor.execute(currentCounterId, command);
    if (!view) {
        return;
    }
    view->setCounterValue(currentCounterId, result.newValue);
    view->appendCommandLog(log);
}

void MainPresenter::onIncrementPressed() {
    const int step = stepFor(currentCounterId);
    domain::CounterCommand command{.kind = domain::CounterCommandKind::Increment, .delta = step};
    executeAndRefresh(command, std::format("Command: +{} (counter {})", step, currentCounterId.getValue()));
}

void MainPresenter::onDecrementPressed() {
    const int step = stepFor(currentCounterId);
    domain::CounterCommand command{.kind = domain::CounterCommandKind::Decrement, .delta = step};
    executeAndRefresh(command, std::format("Command: -{} (counter {})", step, currentCounterId.getValue()));
}

void MainPresenter::onResetPressed() {
    domain::CounterCommand command{.kind = domain::CounterCommandKind::Reset, .delta = 0};
    executeAndRefresh(command, std::format("Command: reset (counter {})", currentCounterId.getValue()));
}

} // namespace presentation
