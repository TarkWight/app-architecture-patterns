#include "ControlChartsTabPresenter.hpp"

namespace presentation::controlChartsTab {

ControlChartsTabPresenter::ControlChartsTabPresenter(Dependencies deps)
    : state(deps.state), setControlChartsTabMinutesUseCase(deps.setControlChartsTabMinutesUseCase),
      setWindImpactUseCase(deps.setWindImpactUseCase), buildControlPlotUseCase(deps.buildControlPlotUseCase) {
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

    view->setMinutes(stateData.controlChartsTabMinutes.value());
    view->setBeaufort(stateData.windImpact.beaufort.value());
    view->setDirection(stateData.windImpact.direction.degrees());
    view->setAngleOfAttack(stateData.windImpact.angleOfAttack.degrees());

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

    updateWindImpact(value, stateData.windImpact.direction.degrees(), stateData.windImpact.angleOfAttack.degrees());

    if (view != nullptr) {
        view->appendLog("Beaufort value updated");
    }
}

void ControlChartsTabPresenter::onDirectionChanged(double value) {
    const auto &stateData = state.get();

    updateWindImpact(stateData.windImpact.beaufort.value(), value, stateData.windImpact.angleOfAttack.degrees());

    if (view != nullptr) {
        view->appendLog("Wind direction updated");
    }
}

void ControlChartsTabPresenter::onAngleOfAttackChanged(double value) {
    const auto &stateData = state.get();

    updateWindImpact(stateData.windImpact.beaufort.value(), stateData.windImpact.direction.degrees(), value);

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

void ControlChartsTabPresenter::updateWindImpact(double beaufort, double direction, double angleOfAttack) {
    domain::WindImpact profile = domain::makeWindImpact(beaufort, direction, angleOfAttack);
    profile.formula = state.get().functionExpression;

    setWindImpactUseCase.execute(profile);
}

} // namespace presentation::controlChartsTab
