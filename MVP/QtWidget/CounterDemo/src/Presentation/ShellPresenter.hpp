#ifndef SHELLPRESENTER_HPP
#define SHELLPRESENTER_HPP

#include "../Application/Session/SessionState.hpp"
#include "../Application/UseCases/BuildControlPlotUseCase.hpp"
#include "../Application/UseCases/SetFunctionExpressionUseCase.hpp"
#include "../Application/UseCases/SetLineColorUseCase.hpp"
#include "../Application/UseCases/StartTestExecutionUseCase.hpp"
#include "../Application/UseCases/StopTestExecutionUseCase.hpp"
#include "../Domain/Plot.hpp"
#include "../Domain/TestExecutionStatus.hpp"

#include "IShellView.hpp"

namespace presentation {

class ShellPresenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::StartTestExecutionUseCase &startTestExecutionUseCase;
        application::useCases::StopTestExecutionUseCase &stopTestExecutionUseCase;
        application::useCases::SetFunctionExpressionUseCase &setFunctionExpressionUseCase;
        application::useCases::SetLineColorUseCase &setLineColorUseCase;
        application::useCases::BuildControlPlotUseCase &buildControlPlotUseCase;
    };

    explicit ShellPresenter(Dependencies deps);

    void attachView(IShellView &view);
    void detachView();

    void onViewReady();
    void onStartPressed();
    void onStateChanged();
    void onStopPressed();
    void onCalculatePressed();

    void onFunctionEdited(std::string expression);
    void onLineColorSelected(domain::RgbColor color);

  private:
    application::session::SessionState &state;
    application::useCases::StartTestExecutionUseCase &startTestExecutionUseCase;
    application::useCases::StopTestExecutionUseCase &stopTestExecutionUseCase;
    application::useCases::SetFunctionExpressionUseCase &setFunctionExpressionUseCase;
    application::useCases::SetLineColorUseCase &setLineColorUseCase;
    application::useCases::BuildControlPlotUseCase &buildControlPlotUseCase;

    IShellView *view{nullptr};

    static std::string formatTimerText(int elapsedSeconds);
    static bool canStart(domain::TestExecutionStatus status);
    static bool canStop(domain::TestExecutionStatus status);

    void refreshFromState();
};

} // namespace presentation

#endif // SHELLPRESENTER_HPP