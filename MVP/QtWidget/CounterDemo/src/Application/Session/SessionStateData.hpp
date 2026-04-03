#ifndef SESSIONSTATEDATA_HPP
#define SESSIONSTATEDATA_HPP

#include "../../Domain/FormulaExpression.hpp"
#include "../../Domain/Plot.hpp"
#include "../../Domain/Poem.hpp"
#include "../../Domain/Time.hpp"
#include "../../Domain/WindProfile.hpp"

namespace application::session {

struct SessionStateData {
    domain::DurationMinutes timerDuration{20};
    domain::ElapsedSeconds elapsed{0};
    bool timerRunning{false};

    domain::FormulaExpression functionExpression{"sin(x)"};
    domain::RgbColor lineColor{255, 0, 0};

    domain::DurationMinutes controlChartsTabMinutes{20};

    domain::WindProfile windProfile{};

    domain::Poem poem{};

    domain::PlotModel telemetryPlot{};
    domain::PlotModel controlPlot{};
};

} // namespace application::session

#endif // SESSIONSTATEDATA_HPP
