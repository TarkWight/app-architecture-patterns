#include "MainPresenter.hpp"
#include <format>

namespace presentation {

MainPresenter::MainPresenter(application::useCases::SwitchCounterUseCase &switchCounterUseCase)
    : useCase(switchCounterUseCase) {
}

void MainPresenter::attachView(IMainView &view) {
    this->view = &view;
}

void MainPresenter::detachView() {
    view = nullptr;
}

int MainPresenter::stepForTab(int tabIndex) const {
    return tabIndex + 1;
}

void MainPresenter::refreshUi(const application::dto::CountersSnapshot &snapshot) {
    if (!view) {
        return;
    }

    if (mode == domain::CounterMode::Global) {
        view->setCounterValue(domain::CounterId{0}, snapshot.global);
        view->setCounterValue(domain::CounterId{1}, snapshot.global);
        view->setCounterValue(domain::CounterId{2}, snapshot.global);
        return;
    }

    view->setCounterValue(domain::CounterId{0}, snapshot.tab0);
    view->setCounterValue(domain::CounterId{1}, snapshot.tab1);
    view->setCounterValue(domain::CounterId{2}, snapshot.tab2);
}

void MainPresenter::onViewReady() {
    if (!view) {
        return;
    }
    refreshUi(useCase.snapshot());
}

void MainPresenter::onTabChanged(int tabIndex) {
    activeTabIndex = tabIndex;
    if (view) {
        view->appendCommandLog(std::format("UI: активная вкладка = {}", tabIndex));
    }
}

void MainPresenter::onModeToggled(bool isGlobal) {
    mode = isGlobal ? domain::CounterMode::Global : domain::CounterMode::PerTab;

    if (view) {
        view->appendCommandLog(
            std::format("UI: режим счётчика = {}", (mode == domain::CounterMode::Global ? "общий" : "по вкладкам")));
    }

    refreshUi(useCase.snapshot());
}

void MainPresenter::executeAndRefresh(const domain::CounterCommand &command, const std::string &logText) {
    const auto snapshot = useCase.execute(mode, activeTabIndex, command);
    refreshUi(snapshot);
    if (view) {
        view->appendCommandLog(logText);
    }
}

void MainPresenter::onIncrementPressed() {
    const int step = stepForTab(activeTabIndex);
    domain::CounterCommand command{.kind = domain::CounterCommandKind::Increment, .delta = step};
    executeAndRefresh(command, std::format("Command: +{} (tab {}, mode {})", step, activeTabIndex,
                                           (mode == domain::CounterMode::Global ? "global" : "per-tab")));
}

void MainPresenter::onDecrementPressed() {
    const int step = stepForTab(activeTabIndex);
    domain::CounterCommand command{.kind = domain::CounterCommandKind::Decrement, .delta = step};
    executeAndRefresh(command, std::format("Command: -{} (tab {}, mode {})", step, activeTabIndex,
                                           (mode == domain::CounterMode::Global ? "global" : "per-tab")));
}

void MainPresenter::onResetPressed() {
    domain::CounterCommand command{.kind = domain::CounterCommandKind::Reset, .delta = 0};
    executeAndRefresh(command, std::format("Command: reset (tab {}, mode {})", activeTabIndex,
                                           (mode == domain::CounterMode::Global ? "global" : "per-tab")));
}

} // namespace presentation
