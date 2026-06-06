#include "TelemetryChartsTabPresenter.hpp"

namespace presentation::telemetryChartsTab {

TelemetryChartsTabPresenter::TelemetryChartsTabPresenter(
    application::useCases::SetTelemetryWindowUseCase &setTelemetryWindowUseCase)
    : setTelemetryWindowUseCase(setTelemetryWindowUseCase) {
}

void TelemetryChartsTabPresenter::attachView(ITelemetryChartsTabView &view) {
    this->view = &view;
}

void TelemetryChartsTabPresenter::detachView() {
    view = nullptr;
}

void TelemetryChartsTabPresenter::onViewReady() {
    if (view != nullptr) {
        view->refreshPlot();
    }
}

void TelemetryChartsTabPresenter::onRebuildPlotPressed() {
    setTelemetryWindowUseCase.followTail();

    if (view != nullptr) {
        view->refreshPlot();
        view->appendLog("Telemetry tail selected");
    }
}

void TelemetryChartsTabPresenter::onTelemetryWindowChanged(int windowEndSeconds) {
    setTelemetryWindowUseCase.execute(static_cast<double>(windowEndSeconds));

    if (view != nullptr) {
        view->refreshPlot();
    }
}

} // namespace presentation::telemetryChartsTab
