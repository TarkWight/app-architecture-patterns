#include "SessionState.hpp"

#include "../Services/TelemetryPlotBuilder.hpp"

#include "../../Domain/AxisId.hpp"
#include "../../Domain/ControlTrace.hpp"
#include "../../Application/Dto/PlotModel.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/Time.hpp"
#include "../../Domain/WindControlProfile.hpp"
#include "../../Domain/WindImpact.hpp"

#include "SessionStateData.hpp"
#include "Subscription.hpp"

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace application::session {
const application::session::SessionStateData &application::session::SessionState::get() const {
    return data;
}

const ExecutionStateData &SessionState::execution() const {
    return data.execution;
}

const ConnectionStateData &SessionState::connection() const {
    return data.connection;
}

const TelemetryStateData &SessionState::telemetry() const {
    return data.telemetry;
}

const ControlStateData &SessionState::control() const {
    return data.control;
}

const ProtocolStateData &SessionState::protocol() const {
    return data.protocol;
}

const ReadinessStateData &SessionState::readiness() const {
    return data.readiness;
}

Subscription application::session::SessionState::subscribe(Listener listener) {
    std::lock_guard lock(mu);
    const auto id = ++lastId;
    listeners.emplace(id, std::move(listener));

    listeners.at(id)(data);

    return Subscription([this, id] {
        std::lock_guard lock(mu);
        listeners.erase(id);
    });
}

void application::session::SessionState::setFunctionExpression(std::string expr) {
    data.control.functionExpression.value = std::move(expr);
    notify();
}

void application::session::SessionState::setWindImpact(domain::WindImpact profile) {
    data.control.windImpact = std::move(profile);
    resetReadinessWithoutNotify();
    notify();
}

void application::session::SessionState::setLineColor(application::dto::RgbColor color) {
    data.control.lineColor = color;
    notify();
}

void application::session::SessionState::setControlChartsTabMinutes(domain::DurationMinutes minutes) {
    data.control.controlChartsTabMinutes = minutes;
    notify();
}

void application::session::SessionState::setTestExecutionStatus(domain::TestExecutionStatus status) {
    data.execution.testExecutionStatus = status;
    notify();
}

void application::session::SessionState::setTestTimeSource(domain::TestTimeSource source) {
    data.protocol.testTimeSource = source;
    resetReadinessWithoutNotify();
    notify();
}

void application::session::SessionState::setTestTimeDirection(domain::TestTimeDirection direction) {
    data.execution.testTimeDirection = direction;
    notify();
}

void application::session::SessionState::setEstimatedTestDurationMinutes(domain::DurationMinutes minutes) {
    data.protocol.estimatedTestDuration = minutes;
    notify();
}

void application::session::SessionState::setOperatorTestDurationMinutes(domain::DurationMinutes minutes) {
    data.protocol.operatorTestDuration = minutes;
    notify();
}

void application::session::SessionState::setActiveTestDurationMinutes(domain::DurationMinutes minutes) {
    data.execution.activeTestDuration = minutes;
    notify();
}

void application::session::SessionState::setElapsedSeconds(domain::ElapsedSeconds seconds) {
    data.execution.elapsed = seconds;
    notify();
}

void application::session::SessionState::setRemainingSeconds(domain::RemainingSeconds seconds) {
    data.execution.remaining = seconds;
    notify();
}

void application::session::SessionState::setTelemetryPlot(application::dto::PlotModel plot) {
    data.telemetry.telemetryPlot = std::move(plot);
    notify();
}

void application::session::SessionState::appendTelemetrySample(domain::AxisTelemetrySample sample) {
    if (!sample.valid) {
        return;
    }

    data.telemetry.telemetryHistory.push_back(sample);

    constexpr std::size_t maxTelemetryHistorySamples = 50'000;
    if (data.telemetry.telemetryHistory.size() > maxTelemetryHistorySamples) {
        const auto eraseCount = data.telemetry.telemetryHistory.size() - maxTelemetryHistorySamples;
        data.telemetry.telemetryHistory.erase(
            data.telemetry.telemetryHistory.begin(),
            data.telemetry.telemetryHistory.begin() +
                static_cast<std::vector<domain::AxisTelemetrySample>::difference_type>(eraseCount));
    }

    if (data.telemetry.telemetryFollowTail && !data.telemetry.telemetryHistory.empty()) {
        data.telemetry.telemetryWindowEndSeconds =
            domain::TelemetryWindowEnd::fromTail(data.telemetry.telemetryHistory.front(), sample);
    }

    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::setTelemetryWindowEnd(domain::TelemetryWindowEnd end) {
    data.telemetry.telemetryFollowTail = false;
    data.telemetry.telemetryWindowEndSeconds = end;

    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::followTelemetryTail() {
    data.telemetry.telemetryFollowTail = true;

    if (!data.telemetry.telemetryHistory.empty()) {
        data.telemetry.telemetryWindowEndSeconds = domain::TelemetryWindowEnd::fromTail(
            data.telemetry.telemetryHistory.front(), data.telemetry.telemetryHistory.back());
    }

    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::setTelemetryAxisColor(domain::AxisId axisId,
                                                               application::dto::RgbColor color) {
    if (axisId == domain::axis0) {
        data.telemetry.telemetryAxisYColor = color;
    } else if (axisId == domain::axis1) {
        data.telemetry.telemetryAxisZColor = color;
    }

    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::setTelemetryAxisVisible(domain::AxisId axisId, bool visible) {
    if (axisId == domain::axis0) {
        data.telemetry.telemetryAxisYVisible = visible;
    } else if (axisId == domain::axis1) {
        data.telemetry.telemetryAxisZVisible = visible;
    }

    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::setStandControlMode(domain::StandControlMode mode) {
    data.control.standControlMode = mode;
    notify();
}

void application::session::SessionState::setTestModeState(domain::TestMode testMode, domain::StandControlMode standMode,
                                                          domain::TestTimeSource timeSource,
                                                          domain::TestTimeDirection timeDirection) {
    data.protocol.testProtocol.testMode = testMode;
    data.control.standControlMode = standMode;
    data.protocol.testTimeSource = timeSource;
    data.execution.testTimeDirection = timeDirection;
    resetReadinessWithoutNotify();
    notify();
}

void application::session::SessionState::setAppliedStandImpact(domain::WindImpact profile) {
    data.control.appliedStandImpact = std::move(profile);
    data.control.windImpact = data.control.appliedStandImpact;
    notify();
}

void application::session::SessionState::setTargetStandImpact(domain::WindImpact profile) {
    data.control.targetStandImpact = std::move(profile);
    resetReadinessWithoutNotify();
    notify();
}

void application::session::SessionState::setControlPlot(application::dto::PlotModel plot) {
    data.control.controlPlot = std::move(plot);
    notify();
}

void application::session::SessionState::setControlProfile(domain::WindControlProfile profile) {
    data.control.controlProfile = std::move(profile);
    notify();
}

void application::session::SessionState::resetTelemetrySession() {
    data.telemetry.telemetryHistory.clear();
    data.telemetry.telemetryFollowTail = true;
    data.telemetry.telemetryWindowEndSeconds = domain::TelemetryWindowEnd::fromSeconds(0.0);
    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::resetControlSession() {
    data.control.controlTrace.clear();
    data.control.controlProfile = domain::WindControlProfile{};
    data.control.controlPlot = application::dto::PlotModel{};
    notify();
}

void application::session::SessionState::clearControlTrace() {
    data.control.controlTrace.clear();
    notify();
}

void application::session::SessionState::appendControlTraceSample(domain::ControlTraceSample sample) {
    data.control.controlTrace.append(std::move(sample));
    notify();
}

void application::session::SessionState::setTestProtocolTitle(std::string title) {
    data.protocol.testProtocol.title = std::move(title);
    notify();
}

void application::session::SessionState::setTestProtocolLine(int idx, std::string line) {
    if (idx < 0 || idx >= 8) {
        return;
    }

    data.protocol.testProtocol.lines[static_cast<std::size_t>(idx)] = std::move(line);
    notify();
}

void application::session::SessionState::setTestProtocolMode(domain::TestMode mode) {
    data.protocol.testProtocol.testMode = mode;
    resetReadinessWithoutNotify();
    notify();
}

void application::session::SessionState::setTestProtocolProgram(domain::TestProgram program) {
    data.protocol.testProtocol.testProgram = program;
    notify();
}

void application::session::SessionState::setTestProtocolDroneParameters(
    std::vector<domain::TestProtocolParameter> parameters) {
    data.protocol.testProtocol.droneParameters = std::move(parameters);
    resetReadinessWithoutNotify();
    notify();
}

void application::session::SessionState::setTestProtocolDroneParameterValue(int idx, std::string value) {
    if (idx < 0 || idx >= static_cast<int>(data.protocol.testProtocol.droneParameters.size())) {
        return;
    }

    data.protocol.testProtocol.droneParameters[static_cast<std::size_t>(idx)].value = std::move(value);
    resetReadinessWithoutNotify();
    notify();
}

void application::session::SessionState::setAxis1State(domain::AxisState stateValue) {
    data.connection.axis1State = stateValue;
    notify();
}

void application::session::SessionState::setAxis2State(domain::AxisState stateValue) {
    data.connection.axis2State = stateValue;
    notify();
}

void application::session::SessionState::setTelemetryStatus(domain::TelemetryStatus status) {
    data.telemetry.telemetryStatus = status;
    notify();
}

void application::session::SessionState::setStandConnectionStatus(domain::StandConnectionStatus status) {
    data.connection.standConnectionStatus = status;
    notify();
}

void application::session::SessionState::setTelemetryPollInterval(domain::TelemetryPollInterval interval) {
    data.connection.telemetryPollInterval = interval;
    notify();
}

void application::session::SessionState::setReadinessFromEstimationResult(
    const domain::EstimatedTestDurationResult &result, domain::WindImpact calculatedForImpact) {
    data.readiness.warnings = result.warnings;
    data.readiness.errors = result.errors;
    data.readiness.values = result.values;
    data.readiness.calculatedForImpact = calculatedForImpact;
    data.readiness.hasCalculatedForImpact = true;

    if (!result.duration.has_value()) {
        data.readiness.status = ReadinessStatus::Failed;
    } else if (!result.errors.empty()) {
        data.readiness.status = ReadinessStatus::Dangerous;
    } else if (!result.warnings.empty()) {
        data.readiness.status = ReadinessStatus::Warning;
    } else {
        data.readiness.status = ReadinessStatus::Ok;
    }

    notify();
}

void application::session::SessionState::resetReadiness() {
    resetReadinessWithoutNotify();
    notify();
}

void application::session::SessionState::notify() {
    std::unordered_map<long long, Listener> copy;
    {
        std::lock_guard lock(mu);
        copy = listeners;
    }

    for (auto &[id, listener] : copy) {
        listener(data);
    }
}

void application::session::SessionState::rebuildTelemetryPlot() {
    data.telemetry.telemetryPlot = application::services::TelemetryPlotBuilder{}.build(data.telemetry);
}

void application::session::SessionState::resetReadinessWithoutNotify() {
    data.readiness = ReadinessStateData{};
}

} // namespace application::session
