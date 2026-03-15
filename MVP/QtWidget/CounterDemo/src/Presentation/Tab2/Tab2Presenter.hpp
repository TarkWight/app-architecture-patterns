#ifndef TAB2PRESENTER_HPP
#define TAB2PRESENTER_HPP

#include "../../Application/UseCases/BuildFormulaPlotUseCase.hpp"
#include "../../Application/UseCases/ExecuteCounterCommandUseCase.hpp"
#include "../../Application/UseCases/SetTab2MinutesUseCase.hpp"
#include "../../Domain/Command.hpp"
#include "../../Domain/CounterId.hpp"

#include "ITab2View.hpp"

namespace presentation::tab2 {

class Tab2Presenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::ExecuteCounterCommandUseCase &executeCounterCommandUseCase;
        application::useCases::SetTab2MinutesUseCase &setTab2MinutesUseCase;
        application::useCases::BuildFormulaPlotUseCase &buildFormulaPlotUseCase;
    };

    explicit Tab2Presenter(Dependencies deps);

    void attachView(ITab2View &view);
    void detachView();

    void onViewReady();

    void onIncrementPressed();
    void onDecrementPressed();
    void onResetPressed();

    void onMinutesChanged(int minutes);
    void onRebuildPlotPressed();

  private:
    static constexpr domain::CounterId counterId{1};

    application::session::SessionState &state;
    application::useCases::ExecuteCounterCommandUseCase &executeCounterCommandUseCase;
    application::useCases::SetTab2MinutesUseCase &setTab2MinutesUseCase;
    application::useCases::BuildFormulaPlotUseCase &buildFormulaPlotUseCase;

    ITab2View *view{nullptr};

    void executeCommand(const domain::CounterCommand &command, const std::string &logText);
};

} // namespace presentation::tab2

#endif // TAB2PRESENTER_HPP
