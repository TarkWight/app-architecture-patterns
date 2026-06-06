#ifndef TELEMETRYCHARTSTABPRESENTER_H
#define TELEMETRYCHARTSTABPRESENTER_H

#include "../../Application/UseCases/SetTelemetryWindowUseCase.hpp"
#include "ITelemetryChartsTabView.hpp"

namespace presentation::telemetryChartsTab {

class TelemetryChartsTabPresenter final {
  public:
    explicit TelemetryChartsTabPresenter(application::useCases::SetTelemetryWindowUseCase &setTelemetryWindowUseCase);

    void attachView(ITelemetryChartsTabView &view);
    void detachView();

    void onViewReady();

    void onRebuildPlotPressed();
    void onTelemetryWindowChanged(int windowEndSeconds);

  private:
    application::useCases::SetTelemetryWindowUseCase &setTelemetryWindowUseCase;

    ITelemetryChartsTabView *view{nullptr};
};

} // namespace presentation::telemetryChartsTab

#endif // TELEMETRYCHARTSTABPRESENTER_H
