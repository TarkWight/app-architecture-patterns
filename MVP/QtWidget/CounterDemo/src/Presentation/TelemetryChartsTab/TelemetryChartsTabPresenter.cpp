#include "TelemetryChartsTabPresenter.hpp"

namespace presentation::telemetryChartsTab {

TelemetryChartsTabPresenter::TelemetryChartsTabPresenter(Dependencies deps)
    : setTelemetryWindowUseCase(deps.setTelemetryWindowUseCase),
      setTelemetryAxisColorUseCase(deps.setTelemetryAxisColorUseCase),
      setTelemetryAxisVisibleUseCase(deps.setTelemetryAxisVisibleUseCase) {
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
    setTelemetryWindowUseCase.execute(domain::TelemetryWindowEnd::fromSeconds(static_cast<double>(windowEndSeconds)));

    if (view != nullptr) {
        view->refreshPlot();
    }
}

void TelemetryChartsTabPresenter::onTelemetryAxisColorSelected(domain::AxisId axisId, domain::RgbColor color) {
    setTelemetryAxisColorUseCase.execute(axisId, color);

    if (view != nullptr) {
        view->refreshPlot();
    }
}

void TelemetryChartsTabPresenter::onTelemetryAxisVisibilityChanged(domain::AxisId axisId, bool visible) {
    setTelemetryAxisVisibleUseCase.execute(axisId, visible);

    if (view != nullptr) {
        view->refreshPlot();
    }
}

} // namespace presentation::telemetryChartsTab
