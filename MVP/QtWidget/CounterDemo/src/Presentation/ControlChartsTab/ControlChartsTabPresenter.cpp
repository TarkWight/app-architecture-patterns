#include "ControlChartsTabPresenter.hpp"

namespace presentation::controlChartsTab {

ControlChartsTabPresenter::ControlChartsTabPresenter(Dependencies deps)
    : state(deps.state), setControlChartsTabMinutesUseCase(deps.setControlChartsTabMinutesUseCase),
      setWindProfileUseCase(deps.setWindProfileUseCase), buildControlPlotUseCase(deps.buildControlPlotUseCase) {
}

void ControlChartsTabPresenter::attachView(IControlChartsTabView &view) {
    this->view = &view;
}

void ControlChartsTabPresenter::detachView() {
    view = nullptr;
}

void ControlChartsTabPresenter::onViewReady() {
    if (view == nullptr) {
        return;
    }

    const auto &stateData = state.get();

    view->setMinutes(stateData.controlChartsTabMinutes.value);
    view->setBeaufort(stateData.windProfile.beaufort.value());
    view->setDirection(stateData.windProfile.direction.degrees());
    view->setAngleOfAttack(stateData.windProfile.angleOfAttack.degrees());

    onRebuildPlotPressed();
}

void ControlChartsTabPresenter::onMinutesChanged(int minutes) {
    setControlChartsTabMinutesUseCase.execute(minutes);

    if (view != nullptr) {
        view->appendLog("ControlChartsTab minutes updated");
    }
}

void ControlChartsTabPresenter::onBeaufortChanged(double value) {
    const auto &stateData = state.get();

    updateWindProfile(value, stateData.windProfile.direction.degrees(), stateData.windProfile.angleOfAttack.degrees());

    if (view != nullptr) {
        view->appendLog("Beaufort value updated");
    }
}

void ControlChartsTabPresenter::onDirectionChanged(double value) {
    const auto &stateData = state.get();

    updateWindProfile(stateData.windProfile.beaufort.value(), value, stateData.windProfile.angleOfAttack.degrees());

    if (view != nullptr) {
        view->appendLog("Wind direction updated");
    }
}

void ControlChartsTabPresenter::onAngleOfAttackChanged(double value) {
    const auto &stateData = state.get();

    updateWindProfile(stateData.windProfile.beaufort.value(), stateData.windProfile.direction.degrees(), value);

    if (view != nullptr) {
        view->appendLog("Angle of attack updated");
    }
}

void ControlChartsTabPresenter::onRebuildPlotPressed() {
    buildControlPlotUseCase.execute();

    if (view != nullptr) {
        view->refreshPlot();
        view->appendLog("ControlChartsTab formula plot rebuilt");
    }
}

void ControlChartsTabPresenter::updateWindProfile(double beaufort, double direction, double angleOfAttack) {
    domain::WindProfile profile = domain::makeWindProfile(beaufort, direction, angleOfAttack);
    profile.formula = state.get().functionExpression;

    setWindProfileUseCase.execute(profile);
}

} // namespace presentation::controlChartsTab
