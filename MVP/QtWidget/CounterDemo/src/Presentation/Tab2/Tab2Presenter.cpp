#include "Tab2Presenter.hpp"

namespace presentation::tab2 {

Tab2Presenter::Tab2Presenter(Dependencies deps)
    : state(deps.state), executeCounterCommandUseCase(deps.executeCounterCommandUseCase),
      setTab2MinutesUseCase(deps.setTab2MinutesUseCase), buildFormulaPlotUseCase(deps.buildFormulaPlotUseCase) {
}

void Tab2Presenter::attachView(ITab2View &view) {
    this->view = &view;
}

void Tab2Presenter::detachView() {
    view = nullptr;
}

void Tab2Presenter::onViewReady() {
    if (view != nullptr) {
        view->setMinutes(state.get().tab2Minutes.value);
    }

    onRebuildPlotPressed();
}

void Tab2Presenter::onIncrementPressed() {
    executeCommand(domain::CounterCommand{.kind = domain::CounterCommandKind::Increment, .delta = 1},
                   "Tab2 counter increment");
}

void Tab2Presenter::onDecrementPressed() {
    executeCommand(domain::CounterCommand{.kind = domain::CounterCommandKind::Decrement, .delta = 1},
                   "Tab2 counter decrement");
}

void Tab2Presenter::onResetPressed() {
    executeCommand(domain::CounterCommand{.kind = domain::CounterCommandKind::Reset, .delta = 0}, "Tab2 counter reset");
}

void Tab2Presenter::onMinutesChanged(int minutes) {
    setTab2MinutesUseCase.execute(minutes);

    if (view != nullptr) {
        view->appendLog("Tab2 minutes updated");
    }
}

void Tab2Presenter::onRebuildPlotPressed() {
    buildFormulaPlotUseCase.execute();

    if (view != nullptr) {
        view->refreshPlot();
        view->appendLog("Tab2 formula plot rebuilt");
    }
}

void Tab2Presenter::executeCommand(const domain::CounterCommand &command, const std::string &logText) {
    const auto result = executeCounterCommandUseCase.execute(counterId, command);

    if (view != nullptr) {
        view->setCounterValue(result.newValue);
        view->appendLog(logText);
    }
}

} // namespace presentation::tab2
