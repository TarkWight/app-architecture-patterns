#include "TestProtocolTabPresenter.hpp"

#include "../../Localization/TestProtocolStrings.hpp"

#include <exception>
#include <utility>

namespace presentation::testProtocolTab {

TestProtocolTabPresenter::TestProtocolTabPresenter(Dependencies deps)
    : state(deps.state), updateTestProtocolUseCase(deps.updateTestProtocolUseCase),
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
        view->appendLog(localization::testProtocol::pdfReportDefaultsLoaded);
    } catch (const std::exception &e) {
        loadPdfReportDefaultsUseCase.applyEmptyDefaults();
        view->appendLog(std::string{localization::testProtocol::pdfReportDefaultsLoadFailedPrefix} + e.what());
    }

    syncViewFromState();
}

void TestProtocolTabPresenter::syncViewFromState() {
    if (view == nullptr) {
        return;
    }

    const auto &session = state.get();

    view->setTestProtocolTitle(session.protocol.testProtocol.title);

    for (int i = 0; i < 8; ++i) {
        view->setTestProtocolLine(i, session.protocol.testProtocol.lines[static_cast<std::size_t>(i)]);
    }

    view->setTestProtocolDroneParameters(session.protocol.testProtocol.droneParameters);
}

void TestProtocolTabPresenter::onTestProtocolTitleChanged(std::string title) {
    updateTestProtocolUseCase.updateTitle(std::move(title));

    if (view != nullptr) {
        view->appendLog(localization::testProtocol::testProtocolTitleUpdated);
    }
}

void TestProtocolTabPresenter::onTestProtocolLineChanged(int index, std::string line) {
    updateTestProtocolUseCase.updateLine(index, std::move(line));

    if (view != nullptr) {
        view->appendLog(localization::testProtocol::testProtocolLineUpdated);
    }
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
        view->appendLog(std::string{localization::testProtocol::pdfReportFieldsLoadedFromTomlPrefix} + filePath);
    } catch (const std::exception &e) {
        view->appendLog(std::string{localization::testProtocol::pdfReportTomlLoadFailedPrefix} + e.what());
    }
}

void TestProtocolTabPresenter::onSavePdfTomlTemplatePressed(const std::string &filePath) {
    if (view == nullptr) {
        return;
    }

    try {
        loadPdfReportDefaultsUseCase.saveEmptyTemplate(filePath);
        view->appendLog(std::string{localization::testProtocol::emptyPdfReportTomlTemplateSavedPrefix} + filePath);
    } catch (const std::exception &e) {
        view->appendLog(std::string{localization::testProtocol::pdfReportTomlTemplateSaveFailedPrefix} + e.what());
    }
}

void TestProtocolTabPresenter::onExportPdfPressed(const std::string &filePath) {
    exportPdfUseCase.execute(filePath);

    if (view != nullptr) {
        view->showExportSuccess(filePath);
        view->appendLog(localization::testProtocol::pdfExported);
    }
}

} // namespace presentation::testProtocolTab
