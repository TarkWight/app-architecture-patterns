#ifndef CONTROLCHARTSPRESENTER_HPP
#define CONTROLCHARTSPRESENTER_HPP

#include "../../Application/UseCases/BuildControlPlotUseCase.hpp"
#include "../../Application/UseCases/SetControlChartsTabMinutesUseCase.hpp"
#include "../../Application/UseCases/SetWindProfileUseCase.hpp"

#include "IControlChartsTabView.hpp"

namespace presentation::controlChartsTab {

class ControlChartsTabPresenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::SetControlChartsTabMinutesUseCase &setControlChartsTabMinutesUseCase;
        application::useCases::SetWindProfileUseCase &setWindProfileUseCase;
        application::useCases::BuildControlPlotUseCase &buildControlPlotUseCase;
    };

    explicit ControlChartsTabPresenter(Dependencies deps);

    void attachView(IControlChartsTabView &view);
    void detachView();

    void onViewReady();

    void onMinutesChanged(int minutes);

    void onBeaufortChanged(double value);
    void onDirectionChanged(double value);
    void onAngleOfAttackChanged(double value);

    void onRebuildPlotPressed();

  private:
    application::session::SessionState &state;
    application::useCases::SetControlChartsTabMinutesUseCase &setControlChartsTabMinutesUseCase;
    application::useCases::SetWindProfileUseCase &setWindProfileUseCase;
    application::useCases::BuildControlPlotUseCase &buildControlPlotUseCase;

    IControlChartsTabView *view{nullptr};

    void updateWindProfile(double beaufort, double direction, double angleOfAttack);
};

} // namespace presentation::controlChartsTab

#endif // CONTROLCHARTSPRESENTER_HPP
