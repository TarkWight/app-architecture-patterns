#ifndef SHELLPRESENTER_HPP
#define SHELLPRESENTER_HPP

#include "../Application/Session/SessionState.hpp"
#include "../Application/UseCases/BuildFormulaPlotUseCase.hpp"
#include "../Application/UseCases/SetFunctionExpressionUseCase.hpp"
#include "../Application/UseCases/SetLineColorUseCase.hpp"
#include "../Application/UseCases/StartTimerUseCase.hpp"
#include "../Application/UseCases/StopTimerUseCase.hpp"
#include "../Domain/Plot.hpp"

#include "IShellView.hpp"

namespace presentation {

class ShellPresenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::StartTimerUseCase &startTimerUseCase;
        application::useCases::StopTimerUseCase &stopTimerUseCase;
        application::useCases::SetFunctionExpressionUseCase &setFunctionExpressionUseCase;
        application::useCases::SetLineColorUseCase &setLineColorUseCase;
        application::useCases::BuildFormulaPlotUseCase &buildFormulaPlotUseCase;
    };

    explicit ShellPresenter(Dependencies deps);

    void attachView(IShellView &view);
    void detachView();

    void onViewReady();
    void onStartPressed();
    void onStopPressed();
    void onCalculatePressed();

    void onFunctionEdited(std::string expression);
    void onLineColorSelected(domain::RgbColor color);

  private:
    application::session::SessionState &state;
    application::useCases::StartTimerUseCase &startTimerUseCase;
    application::useCases::StopTimerUseCase &stopTimerUseCase;
    application::useCases::SetFunctionExpressionUseCase &setFunctionExpressionUseCase;
    application::useCases::SetLineColorUseCase &setLineColorUseCase;
    application::useCases::BuildFormulaPlotUseCase &buildFormulaPlotUseCase;

    IShellView *view{nullptr};

    static std::string formatTimerText(int elapsedSeconds);
    void refreshFromState();
};
} // namespace presentation

#endif // SHELLPRESENTER_HPP
