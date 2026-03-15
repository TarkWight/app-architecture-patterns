#ifndef SESSIONSTATEDATA_HPP
#define SESSIONSTATEDATA_HPP

#include "../../Domain/Plot.hpp"
#include "../../Domain/Poem.hpp"
#include "../../Domain/Time.hpp"

namespace application::session {

struct SessionStateData {
    domain::DurationMinutes timerDuration{20};
    domain::ElapsedSeconds elapsed{0};
    bool timerRunning{false};
    std::string functionExpression{"sin(x)"};
    domain::RgbColor lineColor{255, 0, 0};
    domain::DurationMinutes tab2Minutes{20};
    domain::Poem poem{};
    domain::PlotModel plot1{};
    domain::PlotModel plot2{};
};

} // namespace application::session

#endif // SESSIONSTATEDATA_HPP
