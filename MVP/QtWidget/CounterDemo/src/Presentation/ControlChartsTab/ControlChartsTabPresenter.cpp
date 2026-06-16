#include "ControlChartsTabPresenter.hpp"

#include "../../Domain/TestModeStatePolicy.hpp"

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

    view->setMinutes(stateData.control.controlChartsTabMinutes.value());
    view->setBeaufort(stateData.control.windImpact.beaufort.value());
    view->setDirection(stateData.control.windImpact.direction.degrees());
    view->setAngleOfAttack(stateData.control.windImpact.angleOfAttack.degrees());

    refreshMinutesInputEnabled();
    onRebuildPlotPressed();
}

void ControlChartsTabPresenter::onTimeSettingsChanged() {
    refreshMinutesInputEnabled();
}

void ControlChartsTabPresenter::onMinutesChanged(int minutes) {
    setControlChartsTabMinutesUseCase.execute(domain::DurationMinutes::required(minutes));

    if (view != nullptr) {
        view->appendLog("ControlChartsTab minutes updated");
    }
}

void ControlChartsTabPresenter::onBeaufortChanged(double value) {
    const auto &stateData = state.get();

    updateWindImpact(value, stateData.control.windImpact.direction.degrees(),
                     stateData.control.windImpact.angleOfAttack.degrees());

    if (view != nullptr) {
        view->appendLog("Beaufort value updated");
    }
}

void ControlChartsTabPresenter::onDirectionChanged(double value) {
    const auto &stateData = state.get();

    updateWindImpact(stateData.control.windImpact.beaufort.value(), value,
                     stateData.control.windImpact.angleOfAttack.degrees());

    if (view != nullptr) {
        view->appendLog("Wind direction updated");
    }
}

void ControlChartsTabPresenter::onAngleOfAttackChanged(double value) {
    const auto &stateData = state.get();

    updateWindImpact(stateData.control.windImpact.beaufort.value(), stateData.control.windImpact.direction.degrees(),
                     value);

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
    setWindImpactUseCase.execute(domain::makeWindImpact(beaufort, direction, angleOfAttack));
}

void ControlChartsTabPresenter::refreshMinutesInputEnabled() {
    if (view == nullptr) {
        return;
    }

    const auto &stateData = state.get();
    view->setMinutesInputEnabled(domain::TestModeStatePolicy::operatorDurationInputEnabled(
        stateData.protocol.testProtocol.testMode, stateData.protocol.testTimeSource));
}

} // namespace presentation::controlChartsTab
