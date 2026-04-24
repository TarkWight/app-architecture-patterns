#ifndef SESSIONSTATEDATA_HPP
#define SESSIONSTATEDATA_HPP

#include "../../Domain/AxisState.hpp"
#include "../../Domain/Plot.hpp"
#include "../../Domain/TelemetryStatus.hpp"
#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"
#include "../../Domain/Time.hpp"
#include "../../Domain/WindProfile.hpp"
#include "../../Domain/Expression.hpp"

namespace application::session {

struct SessionStateData {
    // ===== Execution =====
    domain::TestExecutionStatus testExecutionStatus{domain::TestExecutionStatus::Idle};

           // ===== Time model =====
    domain::TestTimeSource testTimeSource{domain::TestTimeSource::FreeRun};
    domain::TestTimeDirection testTimeDirection{domain::TestTimeDirection::CountUp};

    domain::DurationMinutes estimatedTestDuration{20};   // рассчитано программой
    domain::DurationMinutes operatorTestDuration{20};    // введено оператором
    domain::DurationMinutes activeTestDuration{20};      // реально используемое при запуске

    domain::ElapsedSeconds elapsed{};
    domain::RemainingSeconds remaining{};

           // ===== UI / scenario input =====
    domain::Expression functionExpression{};
    domain::WindProfile windProfile{};
    domain::RgbColor lineColor{};
    domain::DurationMinutes controlChartsTabMinutes{20};

           // ===== Plots =====
    domain::PlotModel telemetryPlot{};
    domain::PlotModel controlPlot{};

           // ===== Protocol / report =====
    domain::TestProtocol testProtocol{};

           // ===== Optional runtime/device state =====
    domain::AxisState axis1State{domain::AxisState::Disconnected};
    domain::AxisState axis2State{domain::AxisState::Disconnected};

    domain::TelemetryStatus telemetryStatus{domain::TelemetryStatus::Unavailable};
};

} // namespace application::session

#endif // SESSIONSTATEDATA_HPP