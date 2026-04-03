#include "TelemetryChartsTabPresenter.hpp"

namespace presentation::telemetryChartsTab {

TelemetryChartsTabPresenter::TelemetryChartsTabPresenter(
    application::useCases::GenerateStairPlotUseCase &generateStairPlotUseCase)
    : generateStairPlotUseCase(generateStairPlotUseCase) {
}

void TelemetryChartsTabPresenter::attachView(ITelemetryChartsTabView &view) {
    this->view = &view;
}

void TelemetryChartsTabPresenter::detachView() {
    view = nullptr;
}

void TelemetryChartsTabPresenter::onViewReady() {
    onRebuildPlotPressed();
}

void TelemetryChartsTabPresenter::onRebuildPlotPressed() {
    generateStairPlotUseCase.execute();

    if (view != nullptr) {
        view->refreshPlot();
        view->appendLog("Tab1 stair plot rebuilt");
    }
}

} // namespace presentation::telemetryChartsTab
