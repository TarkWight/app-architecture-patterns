#ifndef CONTROLCHARTSPRESENTER_HPP
#define CONTROLCHARTSPRESENTER_HPP

#include "../../Application/UseCases/BuildControlPlotUseCase.hpp"
#include "../../Application/UseCases/EstimateTestDurationUseCase.hpp"
#include "../../Application/UseCases/SetControlChartsTabMinutesUseCase.hpp"
#include "../../Application/UseCases/SetWindImpactUseCase.hpp"

#include "IControlChartsTabView.hpp"

namespace presentation::controlChartsTab {

class ControlChartsTabPresenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::SetControlChartsTabMinutesUseCase &setControlChartsTabMinutesUseCase;
        application::useCases::SetWindImpactUseCase &setWindImpactUseCase;
        application::useCases::BuildControlPlotUseCase &buildControlPlotUseCase;
        application::useCases::EstimateTestDurationUseCase &estimateTestDurationUseCase;
    };

    explicit ControlChartsTabPresenter(Dependencies deps);

    void attachView(IControlChartsTabView &view);
    void detachView();

    void onViewReady();
    void onTimeSettingsChanged();

    void onMinutesChanged(int minutes);

    void onBeaufortChanged(double value);
    void onDirectionChanged(double value);
    void onAngleOfAttackChanged(double value);

    void onRebuildPlotPressed();
    void onReadinessCalculationPressed();

    [[nodiscard]] static std::string messageForReadinessStatus(application::session::ReadinessStatus status);

  private:
    application::session::SessionState &state;
    application::useCases::SetControlChartsTabMinutesUseCase &setControlChartsTabMinutesUseCase;
    application::useCases::SetWindImpactUseCase &setWindImpactUseCase;
    application::useCases::BuildControlPlotUseCase &buildControlPlotUseCase;
    application::useCases::EstimateTestDurationUseCase &estimateTestDurationUseCase;

    IControlChartsTabView *view{nullptr};

    void updateWindImpact(double beaufort, double direction, double angleOfAttack);
    void refreshMinutesInputEnabled();
    void refreshReadinessCalculationEnabled();
};

} // namespace presentation::controlChartsTab

#endif // CONTROLCHARTSPRESENTER_HPP
