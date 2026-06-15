#include "SessionState.hpp"

#include "../../Domain/AxisId.hpp"
#include "../../Domain/ControlTrace.hpp"
#include "../../Domain/Plot.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/Time.hpp"
#include "../../Domain/WindControlProfile.hpp"
#include "../../Domain/WindImpact.hpp"

#include "SessionStateData.hpp"
#include "Subscription.hpp"

#include <algorithm>
#include <cmath>
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

void application::session::SessionState::setWindImpact(domain::WindImpact profile) {
    data.windImpact = std::move(profile);
    notify();
}

void application::session::SessionState::setLineColor(domain::RgbColor color) {
    data.lineColor = color;
    notify();
}

void application::session::SessionState::setControlChartsTabMinutes(int minutes) {
    setControlChartsTabMinutes(domain::DurationMinutes::required(minutes));
}

void application::session::SessionState::setControlChartsTabMinutes(domain::DurationMinutes minutes) {
    data.controlChartsTabMinutes = minutes;
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

void application::session::SessionState::setEstimatedTestDurationMinutes(domain::DurationMinutes minutes) {
    data.estimatedTestDuration = minutes;
    notify();
}

void application::session::SessionState::setOperatorTestDurationMinutes(domain::DurationMinutes minutes) {
    data.operatorTestDuration = minutes;
    notify();
}

void application::session::SessionState::setActiveTestDurationMinutes(domain::DurationMinutes minutes) {
    data.activeTestDuration = minutes;
    notify();
}

void application::session::SessionState::setElapsedSeconds(domain::ElapsedSeconds seconds) {
    data.elapsed = seconds;
    notify();
}

void application::session::SessionState::setRemainingSeconds(domain::RemainingSeconds seconds) {
    data.remaining = seconds;
    notify();
}

void application::session::SessionState::setTelemetryPlot(domain::PlotModel plot) {
    data.telemetryPlot = std::move(plot);
    notify();
}

void application::session::SessionState::appendTelemetrySample(domain::AxisTelemetrySample sample) {
    if (!sample.valid) {
        return;
    }

    data.telemetryHistory.push_back(sample);

    constexpr std::size_t maxTelemetryHistorySamples = 50'000;
    if (data.telemetryHistory.size() > maxTelemetryHistorySamples) {
        const auto eraseCount = data.telemetryHistory.size() - maxTelemetryHistorySamples;
        data.telemetryHistory.erase(
            data.telemetryHistory.begin(),
            data.telemetryHistory.begin() +
                static_cast<std::vector<domain::AxisTelemetrySample>::difference_type>(eraseCount));
    }

    if (data.telemetryFollowTail && !data.telemetryHistory.empty()) {
        data.telemetryWindowEndSeconds = domain::TelemetryWindowEnd::fromTail(data.telemetryHistory.front(), sample);
    }

    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::setTelemetryWindowEndSeconds(domain::TelemetryWindowEnd endSeconds) {
    data.telemetryFollowTail = false;
    data.telemetryWindowEndSeconds = endSeconds;

    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::followTelemetryTail() {
    data.telemetryFollowTail = true;

    if (!data.telemetryHistory.empty()) {
        data.telemetryWindowEndSeconds =
            domain::TelemetryWindowEnd::fromTail(data.telemetryHistory.front(), data.telemetryHistory.back());
    }

    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::setTelemetryAxisColor(domain::AxisId axisId, domain::RgbColor color) {
    if (axisId == domain::axis0) {
        data.telemetryAxisYColor = color;
    } else if (axisId == domain::axis1) {
        data.telemetryAxisZColor = color;
    }

    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::setTelemetryAxisVisible(domain::AxisId axisId, bool visible) {
    if (axisId == domain::axis0) {
        data.telemetryAxisYVisible = visible;
    } else if (axisId == domain::axis1) {
        data.telemetryAxisZVisible = visible;
    }

    rebuildTelemetryPlot();
    notify();
}

void application::session::SessionState::setStandControlMode(domain::StandControlMode mode) {
    data.standControlMode = mode;
    notify();
}

void application::session::SessionState::setTestModeState(domain::TestMode testMode, domain::StandControlMode standMode,
                                                          domain::TestTimeSource timeSource,
                                                          domain::TestTimeDirection timeDirection) {
    data.testProtocol.testMode = testMode;
    data.standControlMode = standMode;
    data.testTimeSource = timeSource;
    data.testTimeDirection = timeDirection;
    notify();
}

void application::session::SessionState::setAppliedStandImpact(domain::WindImpact profile) {
    data.appliedStandImpact = std::move(profile);
    data.windImpact = data.appliedStandImpact;
    notify();
}

void application::session::SessionState::setTargetStandImpact(domain::WindImpact profile) {
    data.targetStandImpact = std::move(profile);
    notify();
}

void application::session::SessionState::setControlPlot(domain::PlotModel plot) {
    data.controlPlot = std::move(plot);
    notify();
}

void application::session::SessionState::setControlProfile(domain::WindControlProfile profile) {
    data.controlProfile = std::move(profile);
    notify();
}

void application::session::SessionState::clearControlTrace() {
    data.controlTrace.clear();
    notify();
}

void application::session::SessionState::appendControlTraceSample(domain::ControlTraceSample sample) {
    data.controlTrace.append(std::move(sample));
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

void application::session::SessionState::setTestProtocolMode(domain::TestMode mode) {
    data.testProtocol.testMode = mode;
    notify();
}

void application::session::SessionState::setTestProtocolProgram(domain::TestProgram program) {
    data.testProtocol.testProgram = program;
    notify();
}

void application::session::SessionState::setTestProtocolDroneParameters(
    std::vector<domain::TestProtocolParameter> parameters) {
    data.testProtocol.droneParameters = std::move(parameters);
    notify();
}

void application::session::SessionState::setTestProtocolDroneParameterValue(int idx, std::string value) {
    if (idx < 0 || idx >= static_cast<int>(data.testProtocol.droneParameters.size())) {
        return;
    }

    data.testProtocol.droneParameters[static_cast<std::size_t>(idx)].value = std::move(value);
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

void application::session::SessionState::setTelemetryPollInterval(domain::TelemetryPollInterval interval) {
    data.telemetryPollInterval = interval;
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
    domain::PlotModel plot{};
    plot.title = "Telemetry";
    plot.x.label = "seconds";
    plot.y = domain::AxisSpec{.min = -180.0, .max = 360.0, .step = 45.0, .label = "degrees"};

    const double windowSeconds = std::max(1.0, data.telemetryWindowSeconds);
    const double endSeconds = std::max(windowSeconds, data.telemetryWindowEndSeconds.seconds());
    const double startSeconds = std::max(0.0, endSeconds - windowSeconds);

    plot.x =
        domain::AxisSpec{.min = startSeconds, .max = startSeconds + windowSeconds, .step = 10.0, .label = "seconds"};

    domain::NamedSeries axisY{};
    axisY.label = "Ось Y / тангаж";
    axisY.color = data.telemetryAxisYColor;

    domain::NamedSeries axisZ{};
    axisZ.label = "Ось Z / направление";
    axisZ.color = data.telemetryAxisZColor;

    if (!data.telemetryHistory.empty()) {
        const double baseTimestamp = data.telemetryHistory.front().timestampSeconds;

        for (const auto &sample : data.telemetryHistory) {
            const double x = sample.timestampSeconds - baseTimestamp;
            if (x < startSeconds || x > plot.x.max) {
                continue;
            }

            if (sample.axisId == domain::axis0 && data.telemetryAxisYVisible) {
                axisY.series.points.push_back(domain::Point{.x = x, .y = sample.position});
            } else if (sample.axisId == domain::axis1 && data.telemetryAxisZVisible) {
                axisZ.series.points.push_back(domain::Point{.x = x, .y = sample.position});
            }
        }
    }

    if (data.telemetryAxisYVisible) {
        plot.seriesList.push_back(std::move(axisY));
    }

    if (data.telemetryAxisZVisible) {
        plot.seriesList.push_back(std::move(axisZ));
    }

    data.telemetryPlot = std::move(plot);
}

} // namespace application::session
