#include "TestProtocolTabPresenter.hpp"

namespace presentation::testProtocolTab {

TestProtocolTabPresenter::TestProtocolTabPresenter(Dependencies deps)
    : state(deps.state), setTimerDurationUseCase(deps.setTimerDurationUseCase),
      updateTestProtocolUseCase(deps.updateTestProtocolUseCase), exportPdfUseCase(deps.exportPdfUseCase) {
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

    view->setOperatorTestDurationMinutes(session.operatorTestDuration.value);
    view->setTestProtocolTitle(session.testProtocol.title);

    for (int i = 0; i < 8; ++i) {
        view->setTestProtocolLine(i, session.testProtocol.lines[static_cast<std::size_t>(i)]);
    }
}

void TestProtocolTabPresenter::onOperatorTestDurationChanged(int minutes) {
    setTimerDurationUseCase.execute(minutes);

    if (view != nullptr) {
        view->appendLog("Timer duration updated");
    }
}

void TestProtocolTabPresenter::onTestProtocolTitleChanged(std::string title) {
    updateTestProtocolUseCase.updateTitle(std::move(title));

    if (view != nullptr) {
        view->appendLog("TestProtocol title updated");
    }
}

void TestProtocolTabPresenter::onTestProtocolLineChanged(int index, std::string line) {
    updateTestProtocolUseCase.updateLine(index, std::move(line));

    if (view != nullptr) {
        view->appendLog("TestProtocol line updated");
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