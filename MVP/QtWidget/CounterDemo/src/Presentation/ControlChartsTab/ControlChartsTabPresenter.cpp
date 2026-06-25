#include "ControlChartsTabPresenter.hpp"

#include "../../Application/Services/ReadinessDiagnosticMessageBuilder.hpp"
#include "../../Domain/TestModeStatePolicy.hpp"
#include "../../Domain/TestProtocol.hpp"

#include <string>
#include <utility>

namespace presentation::controlChartsTab {

ControlChartsTabPresenter::ControlChartsTabPresenter(Dependencies deps)
    : state(deps.state), setControlChartsTabMinutesUseCase(deps.setControlChartsTabMinutesUseCase),
      setWindImpactUseCase(deps.setWindImpactUseCase),
      setUseAngleOfAttackModelUseCase(deps.setUseAngleOfAttackModelUseCase),
      buildControlPlotUseCase(deps.buildControlPlotUseCase), updateTestProtocolUseCase(deps.updateTestProtocolUseCase) {
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
    view->setUseAngleOfAttackModel(stateData.control.useAngleOfAttackModel);
    view->setTestProtocolMode(std::string{domain::testModeKey(stateData.protocol.testProtocol.testMode)});
    view->setTestProtocolProgram(std::string{domain::testProgramKey(stateData.protocol.testProtocol.testProgram)});

    refreshMinutesInputEnabled();
    refreshDurationDisplay();
}

void ControlChartsTabPresenter::onTimeSettingsChanged() {
    refreshMinutesInputEnabled();
    refreshDurationDisplay();
}

void ControlChartsTabPresenter::onMinutesChanged(int minutes) {
    setControlChartsTabMinutesUseCase.execute(domain::DurationMinutes::required(minutes));

    if (view != nullptr) {
        view->appendLog("ControlChartsTab minutes updated");
    }
}

void ControlChartsTabPresenter::onTestProtocolModeChanged(std::string mode) {
    updateTestProtocolUseCase.updateMode(std::move(mode));
    onTimeSettingsChanged();

    if (view != nullptr) {
        view->appendLog("Test mode updated");
    }
}

void ControlChartsTabPresenter::onTestProtocolProgramChanged(std::string program) {
    updateTestProtocolUseCase.updateProgram(std::move(program));

    if (view != nullptr) {
        view->appendLog("Test program updated");
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

void ControlChartsTabPresenter::onUseAngleOfAttackModelChanged(bool enabled) {
    setUseAngleOfAttackModelUseCase.execute(enabled);

    if (view != nullptr) {
        view->appendLog("Angle of attack model setting updated");
    }
}

void ControlChartsTabPresenter::onRebuildPlotPressed() {
    buildControlPlotUseCase.execute();

    if (view != nullptr) {
        view->refreshPlot();
        view->appendLog("ControlChartsTab formula plot rebuilt");
    }
}

void ControlChartsTabPresenter::onCalculationResultChanged() {
    if (view == nullptr) {
        return;
    }

    const auto message = application::services::ReadinessDiagnosticMessageBuilder::build(state.readiness());
    const auto displayText = message.toDisplayText();
    refreshDurationDisplay();
    view->refreshPlot();
    view->showReadinessMessage(displayText);
    view->appendLog(displayText);
}

void ControlChartsTabPresenter::onDurationStateChanged() {
    refreshDurationDisplay();
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

void ControlChartsTabPresenter::refreshDurationDisplay() {
    if (view == nullptr) {
        return;
    }

    const auto &protocol = state.protocol();
    const bool autoCalculated = protocol.testTimeSource == domain::TestTimeSource::AutoCalculated;
    view->setOperatorDurationVisible(!autoCalculated);
    view->setEstimatedDurationVisible(autoCalculated);

    if (!autoCalculated) {
        return;
    }

    switch (state.readiness().status) {
    case application::session::ReadinessStatus::Failed:
        view->setEstimatedDurationText("Расчёт не выполнен");
        return;
    case application::session::ReadinessStatus::Unknown:
        view->setEstimatedDurationText("Не рассчитано");
        return;
    case application::session::ReadinessStatus::Ok:
    case application::session::ReadinessStatus::Warning:
    case application::session::ReadinessStatus::Dangerous:
        view->setEstimatedDurationText(std::to_string(protocol.estimatedTestDuration.value()) + " мин");
        return;
    }
}

} // namespace presentation::controlChartsTab
