#include "ControlChartsTabPresenter.hpp"

#include "../../Domain/TestModeStatePolicy.hpp"
#include "../../Domain/TestProtocol.hpp"

namespace presentation::controlChartsTab {

ControlChartsTabPresenter::ControlChartsTabPresenter(Dependencies deps)
    : state(deps.state), setControlChartsTabMinutesUseCase(deps.setControlChartsTabMinutesUseCase),
      setWindImpactUseCase(deps.setWindImpactUseCase), buildControlPlotUseCase(deps.buildControlPlotUseCase),
      estimateTestDurationUseCase(deps.estimateTestDurationUseCase) {
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
    refreshReadinessCalculationEnabled();
    onRebuildPlotPressed();
}

void ControlChartsTabPresenter::onTimeSettingsChanged() {
    refreshMinutesInputEnabled();
    refreshReadinessCalculationEnabled();
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

void ControlChartsTabPresenter::onReadinessCalculationPressed() {
    estimateTestDurationUseCase.executeForAutoCalculated();

    if (view == nullptr) {
        return;
    }

    const auto message = messageForReadinessStatus(state.readiness().status);
    view->showReadinessMessage(message);
    view->appendLog(message);
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

void ControlChartsTabPresenter::refreshReadinessCalculationEnabled() {
    if (view == nullptr) {
        return;
    }

    const auto mode = state.protocol().testProtocol.testMode;
    view->setReadinessCalculationEnabled(mode == domain::TestMode::Hybrid || mode == domain::TestMode::Automatic);
}

std::string ControlChartsTabPresenter::messageForReadinessStatus(application::session::ReadinessStatus status) {
    switch (status) {
    case application::session::ReadinessStatus::Ok:
        return "Расчёт готовности выполнен. Испытание допустимо.";
    case application::session::ReadinessStatus::Warning:
        return "Расчёт готовности выполнен. Есть предупреждения.";
    case application::session::ReadinessStatus::Dangerous:
        return "Испытание потенциально опасно.";
    case application::session::ReadinessStatus::Failed:
        return "Расчёт готовности невозможен. Испытание потенциально опасно.";
    case application::session::ReadinessStatus::Unknown:
        return "Расчёт готовности не выполнен.";
    }

    return "Расчёт готовности не выполнен.";
}

} // namespace presentation::controlChartsTab
