#include "Tab3Presenter.hpp"

namespace presentation::tab3 {

Tab3Presenter::Tab3Presenter(Dependencies deps)
    : state(deps.state), executeCounterCommandUseCase(deps.executeCounterCommandUseCase),
      setTimerDurationUseCase(deps.setTimerDurationUseCase), updatePoemUseCase(deps.updatePoemUseCase),
      exportPdfUseCase(deps.exportPdfUseCase) {
}

void Tab3Presenter::attachView(ITab3View &view) {
    this->view = &view;
}

void Tab3Presenter::detachView() {
    view = nullptr;
}

void Tab3Presenter::onViewReady() {
    if (view == nullptr) {
        return;
    }

    const auto &session = state.get();

    view->setTimerDurationMinutes(session.timerDuration.value);
    view->setPoemTitle(session.poem.title);

    for (int i = 0; i < 8; ++i) {
        view->setPoemLine(i, session.poem.lines[static_cast<std::size_t>(i)]);
    }
}

void Tab3Presenter::onIncrementPressed() {
    executeCommand(domain::CounterCommand{.kind = domain::CounterCommandKind::Increment, .delta = 1},
                   "Tab3 counter increment");
}

void Tab3Presenter::onDecrementPressed() {
    executeCommand(domain::CounterCommand{.kind = domain::CounterCommandKind::Decrement, .delta = 1},
                   "Tab3 counter decrement");
}

void Tab3Presenter::onResetPressed() {
    executeCommand(domain::CounterCommand{.kind = domain::CounterCommandKind::Reset, .delta = 0}, "Tab3 counter reset");
}

void Tab3Presenter::onTimerDurationChanged(int minutes) {
    setTimerDurationUseCase.execute(minutes);

    if (view != nullptr) {
        view->appendLog("Timer duration updated");
    }
}

void Tab3Presenter::onPoemTitleChanged(std::string title) {
    updatePoemUseCase.updateTitle(std::move(title));

    if (view != nullptr) {
        view->appendLog("Poem title updated");
    }
}

void Tab3Presenter::onPoemLineChanged(int index, std::string line) {
    updatePoemUseCase.updateLine(index, std::move(line));

    if (view != nullptr) {
        view->appendLog("Poem line updated");
    }
}

void Tab3Presenter::onExportPdfPressed(const std::string &filePath) {
    exportPdfUseCase.execute(filePath);

    if (view != nullptr) {
        view->showExportSuccess(filePath);
        view->appendLog("PDF exported");
    }
}

void Tab3Presenter::executeCommand(const domain::CounterCommand &command, const std::string &logText) {
    const auto result = executeCounterCommandUseCase.execute(counterId, command);

    if (view != nullptr) {
        view->setCounterValue(result.newValue);
        view->appendLog(logText);
    }
}

} // namespace presentation::tab3
