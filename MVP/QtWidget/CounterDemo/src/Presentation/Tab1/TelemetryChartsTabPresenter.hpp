#ifndef TELEMETRYCHARTSTABPRESENTER_H
#define TELEMETRYCHARTSTABPRESENTER_H

#include "../../Application/UseCases/GenerateStairPlotUseCase.hpp"
#include "ITelemetryChartsTabView.hpp"

namespace presentation::tab1 {

class TelemetryChartsTabPresenter final {
  public:
    TelemetryChartsTabPresenter(application::useCases::GenerateStairPlotUseCase &generateStairPlotUseCase);

    void attachView(ITelemetryChartsTabView &view);
    void detachView();

    void onViewReady();

    void onRebuildPlotPressed();

  private:
    application::useCases::GenerateStairPlotUseCase &generateStairPlotUseCase;

    ITelemetryChartsTabView *view{nullptr};
};

} // namespace presentation::tab1

#endif // TELEMETRYCHARTSTABPRESENTER_H
