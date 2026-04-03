#ifndef TAB3PRESENTER_HPP
#define TAB3PRESENTER_HPP

#include "../../Application/UseCases/ExportPdfUseCase.hpp"
#include "../../Application/UseCases/SetTimerDurationUseCase.hpp"
#include "../../Application/UseCases/UpdatePoemUseCase.hpp"
#include "../../Application/Session/SessionState.hpp"

#include "ITestProtocolTabView.hpp"

#include <string>

namespace presentation::testProtocolTab {

class TestProtocolTabPresenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::SetTimerDurationUseCase &setTimerDurationUseCase;
        application::useCases::UpdatePoemUseCase &updatePoemUseCase;
        application::useCases::ExportPdfUseCase &exportPdfUseCase;
    };

    explicit TestProtocolTabPresenter(Dependencies deps);

    void attachView(ITestProtocolTabView &view);
    void detachView();

    void onViewReady();

    void onTimerDurationChanged(int minutes);

    void onPoemTitleChanged(std::string title);
    void onPoemLineChanged(int index, std::string line);

    void onExportPdfPressed(const std::string &filePath);

  private:
    application::session::SessionState &state;
    application::useCases::SetTimerDurationUseCase &setTimerDurationUseCase;
    application::useCases::UpdatePoemUseCase &updatePoemUseCase;
    application::useCases::ExportPdfUseCase &exportPdfUseCase;

    ITestProtocolTabView *view{nullptr};
};

} // namespace presentation::testProtocolTab

#endif // TAB3PRESENTER_HPP
