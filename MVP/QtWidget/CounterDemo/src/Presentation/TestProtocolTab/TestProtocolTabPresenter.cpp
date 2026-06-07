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
        loadPdfReportDefaultsUseCase.applyEmptyDefaults();
        view->appendLog(std::string{"PDF report defaults load failed: "} + e.what());
    }

    syncViewFromState();
}

void TestProtocolTabPresenter::syncViewFromState() {
    if (view == nullptr) {
        return;
    }

    const auto &session = state.get();

    view->setOperatorTestDurationMinutes(session.operatorTestDuration.value);
    view->setTestProtocolTitle(session.testProtocol.title);

    for (int i = 0; i < 8; ++i) {
        view->setTestProtocolLine(i, session.testProtocol.lines[static_cast<std::size_t>(i)]);
    }

    view->setTestProtocolMode(session.testProtocol.testMode);
    view->setTestProtocolProgram(session.testProtocol.testProgram);
    view->setTestProtocolDroneParameters(session.testProtocol.droneParameters);
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

void TestProtocolTabPresenter::onTestProtocolModeChanged(std::string mode) {
    updateTestProtocolUseCase.updateMode(std::move(mode));
}

void TestProtocolTabPresenter::onTestProtocolProgramChanged(std::string program) {
    updateTestProtocolUseCase.updateProgram(std::move(program));
}

void TestProtocolTabPresenter::onTestProtocolDroneParameterChanged(int index, std::string value) {
    updateTestProtocolUseCase.updateDroneParameterValue(index, std::move(value));
}

void TestProtocolTabPresenter::onLoadPdfTomlPressed(const std::string &filePath) {
    if (view == nullptr) {
        return;
    }

    try {
        loadPdfReportDefaultsUseCase.execute(filePath);
        syncViewFromState();
        view->appendLog(std::string{"PDF report fields loaded from TOML: "} + filePath);
    } catch (const std::exception &e) {
        view->appendLog(std::string{"PDF report TOML load failed: "} + e.what());
    }
}

void TestProtocolTabPresenter::onSavePdfTomlTemplatePressed(const std::string &filePath) {
    if (view == nullptr) {
        return;
    }

    try {
        loadPdfReportDefaultsUseCase.saveEmptyTemplate(filePath);
        view->appendLog(std::string{"Empty PDF report TOML template saved: "} + filePath);
    } catch (const std::exception &e) {
        view->appendLog(std::string{"PDF report TOML template save failed: "} + e.what());
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
