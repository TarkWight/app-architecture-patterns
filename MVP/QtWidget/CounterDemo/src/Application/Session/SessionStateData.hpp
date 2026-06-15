#ifndef SESSIONSTATEDATA_HPP
#define SESSIONSTATEDATA_HPP

#include "../../Domain/AxisState.hpp"
#include "../../Domain/AxisTelemetrySample.hpp"
#include "../../Domain/AngleOfAttackOscillationPolicy.hpp"
#include "../../Domain/ControlTrace.hpp"
#include "../../Domain/Plot.hpp"
#include "../../Domain/StandControlMode.hpp"
#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/TelemetryStatus.hpp"
#include "../../Domain/TelemetryPollInterval.hpp"
#include "../../Domain/TelemetryWindow.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"
#include "../../Domain/Time.hpp"
#include "../../Domain/WindControlProfile.hpp"
#include "../../Domain/WindImpact.hpp"
#include "../../Domain/Expression.hpp"

#include <vector>

namespace application::session {

inline domain::PlotModel makeInitialTelemetryPlot() {
    domain::PlotModel plot{};
    plot.title = "Telemetry";
    plot.x = domain::AxisSpec{.min = 0.0, .max = 60.0, .step = 10.0, .label = "seconds"};
    plot.y = domain::AxisSpec{.min = -180.0, .max = 360.0, .step = 45.0, .label = "degrees"};
    return plot;
}

struct SessionStateData {
    // ===== Execution =====
    domain::TestExecutionStatus testExecutionStatus{domain::TestExecutionStatus::Idle};

    // ===== Time model =====
    domain::TestTimeSource testTimeSource{domain::TestTimeSource::FreeRun};
    domain::TestTimeDirection testTimeDirection{domain::TestTimeDirection::CountUp};

    domain::DurationMinutes estimatedTestDuration{domain::DurationMinutes::required(20)}; // рассчитано программой
    domain::DurationMinutes operatorTestDuration{domain::DurationMinutes::required(20)};  // введено оператором
    domain::DurationMinutes activeTestDuration{
        domain::DurationMinutes::required(20)}; // реально используемое при запуске

    domain::ElapsedSeconds elapsed{domain::ElapsedSeconds::from(0)};
    domain::RemainingSeconds remaining{domain::RemainingSeconds::from(0)};

    // ===== UI / scenario input =====
    domain::Expression functionExpression{};
    domain::WindImpact windImpact{};
    domain::RgbColor lineColor{};
    domain::DurationMinutes controlChartsTabMinutes{domain::DurationMinutes::required(20)};

    // ===== Plots =====
    domain::PlotModel telemetryPlot{makeInitialTelemetryPlot()};
    domain::PlotModel controlPlot{};
    domain::WindControlProfile controlProfile{};
    domain::ControlTrace controlTrace{};

    std::vector<domain::AxisTelemetrySample> telemetryHistory{};
    double telemetryWindowSeconds{60.0};
    domain::TelemetryWindowEnd telemetryWindowEndSeconds{domain::TelemetryWindowEnd::fromSeconds(0.0)};
    bool telemetryFollowTail{true};
    domain::RgbColor telemetryAxisYColor{220, 60, 50};
    domain::RgbColor telemetryAxisZColor{40, 110, 210};
    bool telemetryAxisYVisible{true};
    bool telemetryAxisZVisible{true};

    // ===== Stand control =====
    domain::StandControlMode standControlMode{domain::StandControlMode::Manual};
    domain::WindImpact appliedStandImpact{};
    domain::WindImpact targetStandImpact{};
    domain::AngleOfAttackOscillationState angleOfAttackOscillation{};

    // ===== Protocol / report =====
    domain::TestProtocol testProtocol{};

    // ===== Optional runtime/device state =====
    domain::AxisState axis1State{domain::AxisState::Disconnected};
    domain::AxisState axis2State{domain::AxisState::Disconnected};

    domain::TelemetryStatus telemetryStatus{domain::TelemetryStatus::Unavailable};

    domain::StandConnectionStatus standConnectionStatus{domain::StandConnectionStatus::Disconnected};
    domain::TelemetryPollInterval telemetryPollInterval{domain::TelemetryPollInterval::fromMilliseconds(1000)};
};

} // namespace application::session

#endif // SESSIONSTATEDATA_HPP
