#include "TestProtocolTabPresenter.hpp"

namespace presentation::testProtocolTab {

TestProtocolTabPresenter::TestProtocolTabPresenter(Dependencies deps)
    : state(deps.state), setTimerDurationUseCase(deps.setTimerDurationUseCase),
      updatePoemUseCase(deps.updatePoemUseCase), exportPdfUseCase(deps.exportPdfUseCase) {
}

void TestProtocolTabPresenter::attachView(ITestProtocolTabView &view) {
    this->view = &view;
}

void TestProtocolTabPresenter::detachView() {
    view = nullptr;
}

void TestProtocolTabPresenter::onViewReady() {
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

void TestProtocolTabPresenter::onTimerDurationChanged(int minutes) {
    setTimerDurationUseCase.execute(minutes);

    if (view != nullptr) {
        view->appendLog("Timer duration updated");
    }
}

void TestProtocolTabPresenter::onPoemTitleChanged(std::string title) {
    updatePoemUseCase.updateTitle(std::move(title));

    if (view != nullptr) {
        view->appendLog("Poem title updated");
    }
}

void TestProtocolTabPresenter::onPoemLineChanged(int index, std::string line) {
    updatePoemUseCase.updateLine(index, std::move(line));

    if (view != nullptr) {
        view->appendLog("Poem line updated");
    }
}

void TestProtocolTabPresenter::onExportPdfPressed(const std::string &filePath) {
    exportPdfUseCase.execute(filePath);

    if (view != nullptr) {
        view->showExportSuccess(filePath);
        view->appendLog("PDF exported");
    }
}

} // namespace presentation::testProtocolTab
