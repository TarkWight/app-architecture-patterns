#include "Tab1Presenter.hpp"

namespace presentation::tab1 {

Tab1Presenter::Tab1Presenter(application::useCases::ExecuteCounterCommandUseCase &executeCounterCommandUseCase,
                             application::useCases::GenerateStairPlotUseCase &generateStairPlotUseCase)
    : executeCounterCommandUseCase(executeCounterCommandUseCase), generateStairPlotUseCase(generateStairPlotUseCase) {
}

void Tab1Presenter::attachView(ITab1View &view) {
    this->view = &view;
}

void Tab1Presenter::detachView() {
    view = nullptr;
}

void Tab1Presenter::onViewReady() {
    onRebuildPlotPressed();
}

void Tab1Presenter::onIncrementPressed() {
    executeCommand(domain::CounterCommand{.kind = domain::CounterCommandKind::Increment, .delta = 1},
                   "Tab1 counter increment");
}

void Tab1Presenter::onDecrementPressed() {
    executeCommand(domain::CounterCommand{.kind = domain::CounterCommandKind::Decrement, .delta = 1},
                   "Tab1 counter decrement");
}

void Tab1Presenter::onResetPressed() {
    executeCommand(domain::CounterCommand{.kind = domain::CounterCommandKind::Reset, .delta = 0}, "Tab1 counter reset");
}

void Tab1Presenter::onRebuildPlotPressed() {
    generateStairPlotUseCase.execute();

    if (view != nullptr) {
        view->refreshPlot();
        view->appendLog("Tab1 stair plot rebuilt");
    }
}

void Tab1Presenter::executeCommand(const domain::CounterCommand &command, const std::string &logText) {
    const auto result = executeCounterCommandUseCase.execute(counterId, command);

    if (view != nullptr) {
        view->setCounterValue(result.newValue);
        view->appendLog(logText);
    }
}

} // namespace presentation::tab1
