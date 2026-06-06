#ifndef TELEMETRYCHARTSTABPRESENTER_H
#define TELEMETRYCHARTSTABPRESENTER_H

#include "../../Application/UseCases/SetTelemetryAxisColorUseCase.hpp"
#include "../../Application/UseCases/SetTelemetryAxisVisibleUseCase.hpp"
#include "../../Application/UseCases/SetTelemetryWindowUseCase.hpp"
#include "../../Domain/AxisId.hpp"
#include "../../Domain/Plot.hpp"
#include "ITelemetryChartsTabView.hpp"

namespace presentation::telemetryChartsTab {

class TelemetryChartsTabPresenter final {
  public:
    struct Dependencies {
        application::useCases::SetTelemetryWindowUseCase &setTelemetryWindowUseCase;
        application::useCases::SetTelemetryAxisColorUseCase &setTelemetryAxisColorUseCase;
        application::useCases::SetTelemetryAxisVisibleUseCase &setTelemetryAxisVisibleUseCase;
    };

    explicit TelemetryChartsTabPresenter(Dependencies deps);

    void attachView(ITelemetryChartsTabView &view);
    void detachView();

    void onViewReady();

    void onRebuildPlotPressed();
    void onTelemetryWindowChanged(int windowEndSeconds);
    void onTelemetryAxisColorSelected(domain::AxisId axisId, domain::RgbColor color);
    void onTelemetryAxisVisibilityChanged(domain::AxisId axisId, bool visible);

  private:
    application::useCases::SetTelemetryWindowUseCase &setTelemetryWindowUseCase;
    application::useCases::SetTelemetryAxisColorUseCase &setTelemetryAxisColorUseCase;
    application::useCases::SetTelemetryAxisVisibleUseCase &setTelemetryAxisVisibleUseCase;

    ITelemetryChartsTabView *view{nullptr};
};

} // namespace presentation::telemetryChartsTab

#endif // TELEMETRYCHARTSTABPRESENTER_H
