#ifndef TAB2PRESENTER_HPP
#define TAB2PRESENTER_HPP

#include "../../Application/UseCases/BuildFormulaPlotUseCase.hpp"
#include "../../Application/UseCases/SetControlChartsTabMinutesUseCase.hpp"

#include "IControlChartsTabView.hpp"

namespace presentation::controlChartsTab {

class ControlChartsTabPresenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::SetControlChartsTabMinutesUseCase &setControlChartsTabMinutesUseCase;
        application::useCases::BuildFormulaPlotUseCase &buildFormulaPlotUseCase;
    };

    explicit ControlChartsTabPresenter(Dependencies deps);

    void attachView(IControlChartsTabView &view);
    void detachView();

    void onViewReady();

    void onMinutesChanged(int minutes);
    void onRebuildPlotPressed();

  private:
    application::session::SessionState &state;
    application::useCases::SetControlChartsTabMinutesUseCase &setControlChartsTabMinutesUseCase;
    application::useCases::BuildFormulaPlotUseCase &buildFormulaPlotUseCase;

    IControlChartsTabView *view{nullptr};
};

} // namespace presentation::controlChartsTab

#endif // TAB2PRESENTER_HPP
