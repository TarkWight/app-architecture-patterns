#ifndef TAB3PRESENTER_HPP
#define TAB3PRESENTER_HPP

#include "../../Application/UseCases/ExportPdfUseCase.hpp"
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
        application::useCases::ExportPdfUseCase &exportPdfUseCase;
    };

    explicit TestProtocolTabPresenter(Dependencies deps);

    void attachView(ITestProtocolTabView &view);
    void detachView();

    void onViewReady();

    void onOperatorTestDurationChanged(int minutes);

    void onTestProtocolTitleChanged(std::string title);
    void onTestProtocolLineChanged(int index, std::string line);

    void onExportPdfPressed(const std::string &filePath);

private:
    application::session::SessionState &state;
    application::useCases::SetOperatorTestDurationUseCase &setOperatorTestDurationUseCase;
    application::useCases::UpdateTestProtocolUseCase &updateTestProtocolUseCase;
    application::useCases::ExportPdfUseCase &exportPdfUseCase;

    ITestProtocolTabView *view{nullptr};
};

} // namespace presentation::testProtocolTab

#endif // TAB3PRESENTER_HPP