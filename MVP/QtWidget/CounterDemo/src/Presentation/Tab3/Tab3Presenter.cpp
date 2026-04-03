#include "Tab3Presenter.hpp"

namespace presentation::tab3 {

Tab3Presenter::Tab3Presenter(Dependencies deps)
    : state(deps.state), setTimerDurationUseCase(deps.setTimerDurationUseCase),
      updatePoemUseCase(deps.updatePoemUseCase), exportPdfUseCase(deps.exportPdfUseCase) {
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

} // namespace presentation::tab3
