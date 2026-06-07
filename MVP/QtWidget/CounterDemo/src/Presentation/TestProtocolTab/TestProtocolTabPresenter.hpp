#ifndef TAB3PRESENTER_HPP
#define TAB3PRESENTER_HPP

#include "../../Application/UseCases/ExportPdfUseCase.hpp"
#include "../../Application/UseCases/LoadPdfReportDefaultsUseCase.hpp"
#include "../../Application/UseCases/SetOperatorTestDurationUseCase.hpp"
#include "../../Application/UseCases/UpdateTestProtocolUseCase.hpp"
#include "../../Application/Session/SessionState.hpp"

#include "ITestProtocolTabView.hpp"

#include <string>

namespace presentation::testProtocolTab {

class TestProtocolTabPresenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::SetOperatorTestDurationUseCase &setOperatorTestDurationUseCase;
        application::useCases::UpdateTestProtocolUseCase &updateTestProtocolUseCase;
        application::useCases::LoadPdfReportDefaultsUseCase &loadPdfReportDefaultsUseCase;
        application::useCases::ExportPdfUseCase &exportPdfUseCase;
        std::string pdfReportConfigPath;
    };

    explicit TestProtocolTabPresenter(Dependencies deps);

    void attachView(ITestProtocolTabView &view);
    void detachView();

    void onViewReady();

    void onOperatorTestDurationChanged(int minutes);

    void onTestProtocolTitleChanged(std::string title);
    void onTestProtocolLineChanged(int index, std::string line);
    void onTestProtocolModeChanged(std::string mode);
    void onTestProtocolProgramChanged(std::string program);
    void onTestProtocolDroneParameterChanged(int index, std::string value);

    void onLoadPdfTomlPressed(const std::string &filePath);
    void onExportPdfPressed(const std::string &filePath);

  private:
    application::session::SessionState &state;
    application::useCases::SetOperatorTestDurationUseCase &setOperatorTestDurationUseCase;
    application::useCases::UpdateTestProtocolUseCase &updateTestProtocolUseCase;
    application::useCases::LoadPdfReportDefaultsUseCase &loadPdfReportDefaultsUseCase;
    application::useCases::ExportPdfUseCase &exportPdfUseCase;
    std::string pdfReportConfigPath;

    ITestProtocolTabView *view{nullptr};

    void syncViewFromState();
};

} // namespace presentation::testProtocolTab

#endif // TAB3PRESENTER_HPP
