#include "BuildControlPlotUseCase.hpp"

#include <algorithm>
#include <cmath>

namespace application::useCases {

BuildControlPlotUseCase::BuildControlPlotUseCase(session::SessionState &state, const ports::IFunctionEngine &engine)
    : state(state), engine(engine) {
}

domain::PlotModel BuildControlPlotUseCase::execute() {
    const auto &stateData = state.get();

    domain::PlotModel plot{};
    plot.title = "Control chart";
    plot.color = stateData.lineColor;

    const int minutes = stateData.controlChartsTabMinutes.value > 0 ? stateData.controlChartsTabMinutes.value : 20;

    plot.x = domain::AxisSpec{0.0, static_cast<double>(minutes), 1.0, "minutes"};

    plot.y = domain::AxisSpec{0.0, 7.5, 0.5, "Y"};

    constexpr double sampleStep = 0.1;
    const int sampleCount = static_cast<int>(std::floor(static_cast<double>(minutes) / sampleStep));

    plot.series.points.reserve(static_cast<std::size_t>(sampleCount) + 1);

    const double beaufort = stateData.windProfile.beaufort.value();
    const double angleOfAttack = stateData.windProfile.angleOfAttack.degrees();

    const double windFactor = 1.0 + (beaufort * 0.05);
    const double attackOffset = angleOfAttack * 0.01;

    for (int index = 0; index <= sampleCount; ++index) {
        const double x = static_cast<double>(index) * sampleStep;

        const double baseValue = engine.eval(stateData.functionExpression.value, x);

        const double yRaw = (baseValue * windFactor) + attackOffset;
        const double y = std::clamp(yRaw, plot.y.min, plot.y.max);

        plot.series.points.push_back(domain::Point{.x = x, .y = y});
    }

    state.setControlPlot(plot);
    return plot;
}

} // namespace application::useCases
