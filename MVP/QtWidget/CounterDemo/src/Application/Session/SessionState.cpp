#include "SessionState.hpp"

#include "../../Domain/Plot.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/Time.hpp"
#include "../../Domain/WindProfile.hpp"

#include "SessionStateData.hpp"
#include "Subscription.hpp"

#include <algorithm>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

namespace application::session {

const application::session::SessionStateData &application::session::SessionState::get() const {
    return data;
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
    data.functionExpression.value = std::move(expr);
    notify();
}

void application::session::SessionState::setWindProfile(domain::WindProfile profile) {
    data.windProfile = domain::sanitize(std::move(profile));
    notify();
}

void application::session::SessionState::setLineColor(domain::RgbColor color) {
    data.lineColor = color;
    notify();
}

void application::session::SessionState::setControlChartsTabMinutes(int minutes) {
    data.controlChartsTabMinutes.value = std::clamp(minutes, 1, 24 * 60);
    notify();
}

void application::session::SessionState::setTestExecutionStatus(domain::TestExecutionStatus status) {
    data.testExecutionStatus = status;
    notify();
}

void application::session::SessionState::setTestTimeSource(domain::TestTimeSource source) {
    data.testTimeSource = source;
    notify();
}

void application::session::SessionState::setTestTimeDirection(domain::TestTimeDirection direction) {
    data.testTimeDirection = direction;
    notify();
}

void application::session::SessionState::setEstimatedTestDurationMinutes(int minutes) {
    data.estimatedTestDuration.value = std::clamp(minutes, 1, 24 * 60);
    notify();
}

void application::session::SessionState::setOperatorTestDurationMinutes(int minutes) {
    data.operatorTestDuration.value = std::clamp(minutes, 1, 24 * 60);
    notify();
}

void application::session::SessionState::setActiveTestDurationMinutes(int minutes) {
    data.activeTestDuration.value = std::clamp(minutes, 0, 24 * 60);
    notify();
}

void application::session::SessionState::setElapsedSeconds(int seconds) {
    data.elapsed.value = std::max(0, seconds);
    notify();
}

void application::session::SessionState::setRemainingSeconds(int seconds) {
    data.remaining.value = std::max(0, seconds);
    notify();
}

void application::session::SessionState::setTelemetryPlot(domain::PlotModel plot) {
    data.telemetryPlot = std::move(plot);
    notify();
}

void application::session::SessionState::setControlPlot(domain::PlotModel plot) {
    data.controlPlot = std::move(plot);
    notify();
}

void application::session::SessionState::setTestProtocolTitle(std::string title) {
    data.testProtocol.title = std::move(title);
    notify();
}

void application::session::SessionState::setTestProtocolLine(int idx, std::string line) {
    if (idx < 0 || idx >= 8) {
        return;
    }

    data.testProtocol.lines[static_cast<std::size_t>(idx)] = std::move(line);
    notify();
}

void application::session::SessionState::setAxis1State(domain::AxisState stateValue) {
    data.axis1State = stateValue;
    notify();
}

void application::session::SessionState::setAxis2State(domain::AxisState stateValue) {
    data.axis2State = stateValue;
    notify();
}

void application::session::SessionState::setTelemetryStatus(domain::TelemetryStatus status) {
    data.telemetryStatus = status;
    notify();
}

void application::session::SessionState::setStandConnectionStatus(domain::StandConnectionStatus status) {
    data.standConnectionStatus = status;
    notify();
}

void application::session::SessionState::setTelemetryPollIntervalMs(int intervalMs) {
    data.telemetryPollIntervalMs = std::clamp(intervalMs, 20, 60'000);
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

} // namespace application::session
