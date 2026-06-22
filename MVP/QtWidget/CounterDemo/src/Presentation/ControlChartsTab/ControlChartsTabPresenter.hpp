#ifndef CONTROLCHARTSPRESENTER_HPP
#define CONTROLCHARTSPRESENTER_HPP

#include "../../Application/UseCases/BuildControlPlotUseCase.hpp"
#include "../../Application/UseCases/SetControlChartsTabMinutesUseCase.hpp"
#include "../../Application/UseCases/SetWindImpactUseCase.hpp"
#include "../../Application/UseCases/UpdateTestProtocolUseCase.hpp"

#include "IControlChartsTabView.hpp"

namespace presentation::controlChartsTab {

class ControlChartsTabPresenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::SetControlChartsTabMinutesUseCase &setControlChartsTabMinutesUseCase;
        application::useCases::SetWindImpactUseCase &setWindImpactUseCase;
        application::useCases::BuildControlPlotUseCase &buildControlPlotUseCase;
        application::useCases::UpdateTestProtocolUseCase &updateTestProtocolUseCase;
    };

    explicit ControlChartsTabPresenter(Dependencies deps);

    void attachView(IControlChartsTabView &view);
    void detachView();

    void onViewReady();
    void onTimeSettingsChanged();

    void onMinutesChanged(int minutes);
    void onTestProtocolModeChanged(std::string mode);
    void onTestProtocolProgramChanged(std::string program);

    void onBeaufortChanged(double value);
    void onDirectionChanged(double value);
    void onAngleOfAttackChanged(double value);

    void onRebuildPlotPressed();
    void onCalculationResultChanged();
    void onDurationStateChanged();

  private:
    application::session::SessionState &state;
    application::useCases::SetControlChartsTabMinutesUseCase &setControlChartsTabMinutesUseCase;
    application::useCases::SetWindImpactUseCase &setWindImpactUseCase;
    application::useCases::BuildControlPlotUseCase &buildControlPlotUseCase;
    application::useCases::UpdateTestProtocolUseCase &updateTestProtocolUseCase;

    IControlChartsTabView *view{nullptr};

    void updateWindImpact(double beaufort, double direction, double angleOfAttack);
    void refreshMinutesInputEnabled();
    void refreshDurationDisplay();
};

} // namespace presentation::controlChartsTab

#endif // CONTROLCHARTSPRESENTER_HPP
