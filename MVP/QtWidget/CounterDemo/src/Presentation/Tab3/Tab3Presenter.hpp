#ifndef TAB3PRESENTER_HPP
#define TAB3PRESENTER_HPP

#include "../../Application/UseCases/ExecuteCounterCommandUseCase.hpp"
#include "../../Application/UseCases/ExportPdfUseCase.hpp"
#include "../../Application/UseCases/SetTimerDurationUseCase.hpp"
#include "../../Application/UseCases/UpdatePoemUseCase.hpp"
#include "../../Application/Session/SessionState.hpp"
#include "../../Domain/Command.hpp"
#include "../../Domain/CounterId.hpp"

#include "ITab3View.hpp"

#include <string>

namespace presentation::tab3 {

class Tab3Presenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::ExecuteCounterCommandUseCase &executeCounterCommandUseCase;
        application::useCases::SetTimerDurationUseCase &setTimerDurationUseCase;
        application::useCases::UpdatePoemUseCase &updatePoemUseCase;
        application::useCases::ExportPdfUseCase &exportPdfUseCase;
    };

    explicit Tab3Presenter(Dependencies deps);

    void attachView(ITab3View &view);
    void detachView();

    void onViewReady();

    void onIncrementPressed();
    void onDecrementPressed();
    void onResetPressed();

    void onTimerDurationChanged(int minutes);

    void onPoemTitleChanged(std::string title);
    void onPoemLineChanged(int index, std::string line);

    void onExportPdfPressed(const std::string &filePath);

  private:
    static constexpr domain::CounterId counterId{2};

    application::session::SessionState &state;
    application::useCases::ExecuteCounterCommandUseCase &executeCounterCommandUseCase;
    application::useCases::SetTimerDurationUseCase &setTimerDurationUseCase;
    application::useCases::UpdatePoemUseCase &updatePoemUseCase;
    application::useCases::ExportPdfUseCase &exportPdfUseCase;

    ITab3View *view{nullptr};

    void executeCommand(const domain::CounterCommand &command, const std::string &logText);
};

} // namespace presentation::tab3

#endif // TAB3PRESENTER_HPP
