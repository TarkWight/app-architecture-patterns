#ifndef TAB3PRESENTER_HPP
#define TAB3PRESENTER_HPP

#include "../../Application/UseCases/ExportPdfUseCase.hpp"
#include "../../Application/UseCases/SetTimerDurationUseCase.hpp"
#include "../../Application/UseCases/UpdatePoemUseCase.hpp"
#include "../../Application/Session/SessionState.hpp"

#include "ITab3View.hpp"

#include <string>

namespace presentation::tab3 {

class Tab3Presenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::SetTimerDurationUseCase &setTimerDurationUseCase;
        application::useCases::UpdatePoemUseCase &updatePoemUseCase;
        application::useCases::ExportPdfUseCase &exportPdfUseCase;
    };

    explicit Tab3Presenter(Dependencies deps);

    void attachView(ITab3View &view);
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

    ITab3View *view{nullptr};
};

} // namespace presentation::tab3

#endif // TAB3PRESENTER_HPP
