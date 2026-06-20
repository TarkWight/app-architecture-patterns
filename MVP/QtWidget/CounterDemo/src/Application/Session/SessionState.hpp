#ifndef SESSIONSTATE_HPP
#define SESSIONSTATE_HPP

#include "../../Domain/ControlTrace.hpp"
#include "../../Application/Dto/PlotModel.hpp"
#include "../../Domain/StandControlMode.hpp"
#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"
#include "../../Domain/TelemetryWindow.hpp"
#include "../../Domain/WindControlProfile.hpp"
#include "../../Domain/AxisTelemetrySample.hpp"
#include "../../Domain/HybridBeaufortOverride.hpp"

#include "SessionStateData.hpp"
#include "Subscription.hpp"

#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace application::session {

// SessionState is an in-memory application state container for the main Qt thread.
// get() returns a reference under this single-thread contract; if worker threads are
// introduced, state updates must be routed through queued snapshots on the UI thread.
class SessionState {
  public:
    using Listener = std::function<void(const SessionStateData &)>;

    const SessionStateData &get() const;
    const ExecutionStateData &execution() const;
    const ConnectionStateData &connection() const;
    const TelemetryStateData &telemetry() const;
    const ControlStateData &control() const;
    const ProtocolStateData &protocol() const;
    const ReadinessStateData &readiness() const;

    Subscription subscribe(Listener listener);

    void setFunctionExpression(std::string expr);
    void setWindImpact(domain::WindImpact profile);
    void setLineColor(application::dto::RgbColor color);
    void setControlChartsTabMinutes(domain::DurationMinutes minutes);

    void setTestExecutionStatus(domain::TestExecutionStatus status);

    void setTestTimeSource(domain::TestTimeSource source);
    void setTestTimeDirection(domain::TestTimeDirection direction);

    void setEstimatedTestDurationMinutes(domain::DurationMinutes minutes);
    void setOperatorTestDurationMinutes(domain::DurationMinutes minutes);
    void setActiveTestDurationMinutes(domain::DurationMinutes minutes);

    void setElapsedSeconds(domain::ElapsedSeconds seconds);
    void setRemainingSeconds(domain::RemainingSeconds seconds);

    void setTelemetryPlot(application::dto::PlotModel plot);
    void setControlPlot(application::dto::PlotModel plot);
    void setControlProfile(domain::WindControlProfile profile);
    void resetTelemetrySession();
    void resetControlSession();
    void clearControlTrace();
    void appendControlTraceSample(domain::ControlTraceSample sample);
    void appendTelemetrySample(domain::AxisTelemetrySample sample);
    void setTelemetryWindowEnd(domain::TelemetryWindowEnd end);
    void followTelemetryTail();
    void setTelemetryAxisColor(domain::AxisId axisId, application::dto::RgbColor color);
    void setTelemetryAxisVisible(domain::AxisId axisId, bool visible);
    void setStandControlMode(domain::StandControlMode mode);
    void setTestModeState(domain::TestMode testMode, domain::StandControlMode standMode,
                          domain::TestTimeSource timeSource, domain::TestTimeDirection timeDirection);
    void setAppliedStandImpact(domain::WindImpact profile);
    void setTargetStandImpact(domain::WindImpact profile);
    void setHybridBeaufortOverride(std::optional<domain::HybridBeaufortOverride> overrideState);
    void clearHybridBeaufortOverride();
    void setHybridOperatorDirection(domain::WindDirection direction);
    void setHybridOperatorAngleOfAttack(domain::AngleOfAttack angleOfAttack);

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
    void setTelemetryPollInterval(domain::TelemetryPollInterval interval);
    void setReadinessFromEstimationResult(const domain::EstimatedTestDurationResult &result,
                                          domain::WindImpact calculatedForImpact,
                                          bool calculatedForWorstCaseScenario = false);
    void resetReadiness();

  private:
    SessionStateData data{};

    mutable std::mutex mu{};
    std::unordered_map<long long, Listener> listeners{};
    long long lastId{0};

    void notify();
    void rebuildTelemetryPlot();
    void resetReadinessWithoutNotify();
};

} // namespace application::session

#endif // SESSIONSTATE_HPP
