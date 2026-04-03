#include "Tab1Presenter.hpp"

namespace presentation::tab1 {

Tab1Presenter::Tab1Presenter(application::useCases::GenerateStairPlotUseCase &generateStairPlotUseCase)
    : generateStairPlotUseCase(generateStairPlotUseCase) {
}

void Tab1Presenter::attachView(ITelemetryChartsTabView &view) {
    this->view = &view;
}

void Tab1Presenter::detachView() {
    view = nullptr;
}

void Tab1Presenter::onViewReady() {
    onRebuildPlotPressed();
}

void Tab1Presenter::onRebuildPlotPressed() {
    generateStairPlotUseCase.execute();

    if (view != nullptr) {
        view->refreshPlot();
        view->appendLog("Tab1 stair plot rebuilt");
    }
}

} // namespace presentation::tab1
