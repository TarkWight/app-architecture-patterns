#ifndef SESSIONSTATE_HPP
#define SESSIONSTATE_HPP

#include "../../Domain/Plot.hpp"
#include "../../Domain/StandControlMode.hpp"
#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"
#include "../../Domain/WindControlProfile.hpp"
#include "../../Domain/AxisTelemetrySample.hpp"

#include "SessionStateData.hpp"
#include "Subscription.hpp"

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace application::session {

class SessionState {
  public:
    using Listener = std::function<void(const SessionStateData &)>;

    const SessionStateData &get() const;

    Subscription subscribe(Listener listener);

    void setFunctionExpression(std::string expr);
    void setWindProfile(domain::WindProfile profile);
    void setLineColor(domain::RgbColor color);
    void setControlChartsTabMinutes(int minutes);

    void setTestExecutionStatus(domain::TestExecutionStatus status);

    void setTestTimeSource(domain::TestTimeSource source);
    void setTestTimeDirection(domain::TestTimeDirection direction);

    void setEstimatedTestDurationMinutes(int minutes);
    void setOperatorTestDurationMinutes(int minutes);
    void setActiveTestDurationMinutes(int minutes);

    void setElapsedSeconds(int seconds);
    void setRemainingSeconds(int seconds);

    void setTelemetryPlot(domain::PlotModel plot);
    void setControlPlot(domain::PlotModel plot);
    void setControlProfile(domain::WindControlProfile profile);
    void appendTelemetrySample(domain::AxisTelemetrySample sample);
    void setTelemetryWindowEndSeconds(double endSeconds);
    void followTelemetryTail();
    void setTelemetryAxisColor(domain::AxisId axisId, domain::RgbColor color);
    void setTelemetryAxisVisible(domain::AxisId axisId, bool visible);
    void setStandControlMode(domain::StandControlMode mode);
    void setAppliedStandImpact(domain::WindProfile profile);
    void setTargetStandImpact(domain::WindProfile profile);

    void setTestProtocolTitle(std::string title);
    void setTestProtocolLine(int idx, std::string line);
    void setTestProtocolMode(domain::TestMode mode);
    void setTestProtocolProgram(domain::TestProgram program);
    void setTestProtocolDroneParameters(std::vector<domain::TestProtocolParameter> parameters);
    void setTestProtocolDroneParameterValue(int idx, std::string value);

    void setAxis1State(domain::AxisState state);
    void setAxis2State(domain::AxisState state);
    void setTelemetryStatus(domain::TelemetryStatus status);
    void setStandConnectionStatus(domain::StandConnectionStatus status);
    void setTelemetryPollIntervalMs(int intervalMs);

  private:
    SessionStateData data{};

    mutable std::mutex mu{};
    std::unordered_map<long long, Listener> listeners{};
    long long lastId{0};

    void notify();
    void rebuildTelemetryPlot();
};

} // namespace application::session

#endif // SESSIONSTATE_HPP
