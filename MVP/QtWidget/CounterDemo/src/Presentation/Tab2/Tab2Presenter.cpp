#include "Tab2Presenter.hpp"

namespace presentation::tab2 {

Tab2Presenter::Tab2Presenter(Dependencies deps)
    : state(deps.state), setTab2MinutesUseCase(deps.setTab2MinutesUseCase),
      buildFormulaPlotUseCase(deps.buildFormulaPlotUseCase) {
}

void Tab2Presenter::attachView(ITab2View &view) {
    this->view = &view;
}

void Tab2Presenter::detachView() {
    view = nullptr;
}

void Tab2Presenter::onViewReady() {
    if (view != nullptr) {
        view->setMinutes(state.get().tab2Minutes.value);
    }

    onRebuildPlotPressed();
}

void Tab2Presenter::onMinutesChanged(int minutes) {
    setTab2MinutesUseCase.execute(minutes);

    if (view != nullptr) {
        view->appendLog("Tab2 minutes updated");
    }
}

void Tab2Presenter::onRebuildPlotPressed() {
    buildFormulaPlotUseCase.execute();

    if (view != nullptr) {
        view->refreshPlot();
        view->appendLog("Tab2 formula plot rebuilt");
    }
}

} // namespace presentation::tab2
