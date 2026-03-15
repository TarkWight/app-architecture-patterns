#include "BuildFormulaPlotUseCase.hpp"

namespace application::useCases {

BuildFormulaPlotUseCase::BuildFormulaPlotUseCase(session::SessionState &state, const ports::IFunctionEngine &engine)
    : state(state), engine(engine) {
}

domain::PlotModel application::useCases::BuildFormulaPlotUseCase::execute() {
    const auto &stateData = state.get();

    domain::PlotModel plot{};
    plot.title = "Formula plot";
    plot.color = stateData.lineColor;

    const int minutes = stateData.tab2Minutes.value > 0 ? stateData.tab2Minutes.value : 20;
    plot.x = domain::AxisSpec{0.0, static_cast<double>(minutes), 1.0, "minutes"};

    plot.y = domain::AxisSpec{0.0, 7.5, 0.5, "Y"};

    plot.series.points.reserve(static_cast<std::size_t>(minutes) + 1);

    for (int time = 0; time <= minutes; ++time) {
        const double x = static_cast<double>(time);
        const double yRaw = engine.eval(stateData.functionExpression, x);

        const double y = (yRaw < plot.y.min) ? plot.y.min : (yRaw > plot.y.max ? plot.y.max : yRaw);
        plot.series.points.push_back(domain::Point{x, y});
    }

    state.setPlot2(plot);
    return plot;
}

} // namespace application::useCases
