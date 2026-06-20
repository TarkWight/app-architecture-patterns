#ifndef SESSIONSTATEDATA_HPP
#define SESSIONSTATEDATA_HPP

#include "../../Domain/AxisState.hpp"
#include "../../Domain/AxisTelemetrySample.hpp"
#include "../../Domain/ControlTrace.hpp"
#include "../../Application/Dto/PlotModel.hpp"
#include "../../Domain/StandControlMode.hpp"
#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/TelemetryStatus.hpp"
#include "../../Domain/TelemetryPollInterval.hpp"
#include "../../Domain/TelemetryWindow.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestDurationEstimator.hpp"
#include "../../Domain/HybridBeaufortOverride.hpp"
#include "../../Domain/SafeWindImpactLimitEstimator.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"
#include "../../Domain/Time.hpp"
#include "../../Domain/WindControlProfile.hpp"
#include "../../Domain/WindImpact.hpp"
#include "../../Domain/Expression.hpp"

#include <vector>
#include <optional>

namespace application::session {

enum class ReadinessStatus { Unknown, Ok, Warning, Dangerous, Failed };

inline application::dto::PlotModel makeInitialTelemetryPlot() {
    application::dto::PlotModel plot{};
    plot.title = "Telemetry";
    plot.x = application::dto::AxisSpec{.min = 0.0, .max = 60.0, .step = 10.0, .label = "seconds"};
    plot.y = application::dto::AxisSpec{.min = -180.0, .max = 360.0, .step = 45.0, .label = "degrees"};
    return plot;
}

struct ExecutionStateData {
    domain::TestExecutionStatus testExecutionStatus{domain::TestExecutionStatus::Idle};

    domain::TestTimeDirection testTimeDirection{domain::TestTimeDirection::CountUp};
    domain::DurationMinutes activeTestDuration{
        domain::DurationMinutes::required(20)}; // реально используемое при запуске

    domain::ElapsedSeconds elapsed{domain::ElapsedSeconds::from(0)};
    domain::RemainingSeconds remaining{domain::RemainingSeconds::from(0)};
};

struct ProtocolStateData {
    domain::TestProtocol testProtocol{};
    domain::TestTimeSource testTimeSource{domain::TestTimeSource::FreeRun};
    domain::DurationMinutes estimatedTestDuration{domain::DurationMinutes::required(20)}; // рассчитано программой
    domain::DurationMinutes operatorTestDuration{domain::DurationMinutes::required(20)};  // введено оператором
};

struct ReadinessStateData {
    ReadinessStatus status{ReadinessStatus::Unknown};
    std::vector<domain::TestDurationDiagnostic> warnings{};
    std::vector<domain::TestDurationDiagnostic> errors{};
    domain::TestDurationDiagnosticValues values{};
    domain::WindImpact calculatedForImpact{};
    bool hasCalculatedForImpact{false};
    bool calculatedForWorstCaseScenario{false};
    domain::SafeWindImpactLimitResult safeLimits{};
};

struct ControlStateData {
    domain::Expression functionExpression{};
    domain::WindImpact windImpact{};
    application::dto::RgbColor lineColor{};
    domain::DurationMinutes controlChartsTabMinutes{domain::DurationMinutes::required(20)};

    application::dto::PlotModel controlPlot{};
    domain::WindControlProfile controlProfile{};
    domain::ControlTrace controlTrace{};

    domain::StandControlMode standControlMode{domain::StandControlMode::Manual};
    domain::WindImpact appliedStandImpact{};
    domain::WindImpact targetStandImpact{};
    std::optional<domain::HybridBeaufortOverride> hybridBeaufortOverride{};
    domain::WindDirection hybridOperatorDirection{domain::WindDirection::from(0.0)};
    domain::AngleOfAttack hybridOperatorAngleOfAttack{domain::AngleOfAttack::from(0.0)};
};

struct TelemetryStateData {
    application::dto::PlotModel telemetryPlot{makeInitialTelemetryPlot()};
    std::vector<domain::AxisTelemetrySample> telemetryHistory{};
    double telemetryWindowSeconds{60.0};
    domain::TelemetryWindowEnd telemetryWindowEndSeconds{domain::TelemetryWindowEnd::fromSeconds(0.0)};
    bool telemetryFollowTail{true};
    application::dto::RgbColor telemetryAxisYColor{220, 60, 50};
    application::dto::RgbColor telemetryAxisZColor{40, 110, 210};
    bool telemetryAxisYVisible{true};
    bool telemetryAxisZVisible{true};

    domain::TelemetryStatus telemetryStatus{domain::TelemetryStatus::Unavailable};
};

struct ConnectionStateData {
    domain::AxisState axis1State{domain::AxisState::Disconnected};
    domain::AxisState axis2State{domain::AxisState::Disconnected};

    domain::StandConnectionStatus standConnectionStatus{domain::StandConnectionStatus::Disconnected};
    domain::TelemetryPollInterval telemetryPollInterval{domain::TelemetryPollInterval::fromMilliseconds(1000)};
};

struct SessionStateData {
    ExecutionStateData execution{};
    ConnectionStateData connection{};
    TelemetryStateData telemetry{};
    ControlStateData control{};
    ProtocolStateData protocol{};
    ReadinessStateData readiness{};
};

} // namespace application::session

#endif // SESSIONSTATEDATA_HPP
