#ifndef TAB1PRESENTER_HPP
#define TAB1PRESENTER_HPP

#include "../../Application/UseCases/ExecuteCounterCommandUseCase.hpp"
#include "../../Application/UseCases/GenerateStairPlotUseCase.hpp"
#include "../../Domain/Command.hpp"
#include "../../Domain/CounterId.hpp"

#include "ITab1View.hpp"

namespace presentation::tab1 {

class Tab1Presenter final {
  public:
    Tab1Presenter(application::useCases::ExecuteCounterCommandUseCase &executeCounterCommandUseCase,
                  application::useCases::GenerateStairPlotUseCase &generateStairPlotUseCase);

    void attachView(ITab1View &view);
    void detachView();

    void onViewReady();

    void onIncrementPressed();
    void onDecrementPressed();
    void onResetPressed();

    void onRebuildPlotPressed();

  private:
    static constexpr domain::CounterId counterId{0};

    application::useCases::ExecuteCounterCommandUseCase &executeCounterCommandUseCase;
    application::useCases::GenerateStairPlotUseCase &generateStairPlotUseCase;

    ITab1View *view{nullptr};

    void executeCommand(const domain::CounterCommand &command, const std::string &logText);
};

} // namespace presentation::tab1

#endif // TAB1PRESENTER_HPP
