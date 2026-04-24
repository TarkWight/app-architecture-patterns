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
    view->setBeaufort(stateData.windProfile.beaufort);
    view->setDirection(stateData.windProfile.direction);
    view->setAngleOfAttack(stateData.windProfile.angleOfAttack);

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

    updateWindProfile(value, stateData.windProfile.direction, stateData.windProfile.angleOfAttack);

    if (view != nullptr) {
        view->appendLog("Beaufort value updated");
    }
}

void ControlChartsTabPresenter::onDirectionChanged(double value) {
    const auto &stateData = state.get();

    updateWindProfile(stateData.windProfile.beaufort, value, stateData.windProfile.angleOfAttack);

    if (view != nullptr) {
        view->appendLog("Wind direction updated");
    }
}

void ControlChartsTabPresenter::onAngleOfAttackChanged(double value) {
    const auto &stateData = state.get();

    updateWindProfile(stateData.windProfile.beaufort, stateData.windProfile.direction, value);

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
    domain::WindProfile profile{};
    profile.beaufort = beaufort;
    profile.direction = direction;
    profile.angleOfAttack = angleOfAttack;
    profile.formula = state.get().functionExpression;

    setWindProfileUseCase.execute(profile);
}

} // namespace presentation::controlChartsTab
