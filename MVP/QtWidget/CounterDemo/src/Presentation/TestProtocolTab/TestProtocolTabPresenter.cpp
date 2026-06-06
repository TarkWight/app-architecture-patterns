#include "TestProtocolTabPresenter.hpp"

#include <exception>
#include <utility>

namespace presentation::testProtocolTab {

TestProtocolTabPresenter::TestProtocolTabPresenter(Dependencies deps)
    : state(deps.state), setOperatorTestDurationUseCase(deps.setOperatorTestDurationUseCase),
      updateTestProtocolUseCase(deps.updateTestProtocolUseCase),
      loadPdfReportDefaultsUseCase(deps.loadPdfReportDefaultsUseCase), exportPdfUseCase(deps.exportPdfUseCase),
      pdfReportConfigPath(std::move(deps.pdfReportConfigPath)) {
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

    try {
        loadPdfReportDefaultsUseCase.execute(pdfReportConfigPath);
        view->appendLog("PDF report defaults loaded");
    } catch (const std::exception &e) {
        view->appendLog(std::string{"PDF report defaults load failed: "} + e.what());
    }

    const auto &session = state.get();

    view->setOperatorTestDurationMinutes(session.operatorTestDuration.value);
    view->setTestProtocolTitle(session.testProtocol.title);

    for (int i = 0; i < 8; ++i) {
        view->setTestProtocolLine(i, session.testProtocol.lines[static_cast<std::size_t>(i)]);
    }
}

void TestProtocolTabPresenter::onOperatorTestDurationChanged(int minutes) {
    setOperatorTestDurationUseCase.execute(minutes);

    if (view != nullptr) {
        view->appendLog("Operator test duration updated");
    }
}

void TestProtocolTabPresenter::onTestProtocolTitleChanged(std::string title) {
    updateTestProtocolUseCase.updateTitle(std::move(title));

    if (view != nullptr) {
        view->appendLog("Test protocol title updated");
    }
}

void TestProtocolTabPresenter::onTestProtocolLineChanged(int index, std::string line) {
    updateTestProtocolUseCase.updateLine(index, std::move(line));

    if (view != nullptr) {
        view->appendLog("Test protocol line updated");
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
