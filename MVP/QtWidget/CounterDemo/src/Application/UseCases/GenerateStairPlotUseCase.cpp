#include "GenerateStairPlotUseCase.hpp"

namespace application::useCases {

GenerateStairPlotUseCase::GenerateStairPlotUseCase(application::session::SessionState &state) : state(state) {
}

domain::PlotModel GenerateStairPlotUseCase::execute() {
    const auto &session = state.get();

    domain::PlotModel plot{};
    plot.title = "Stair plot";
    plot.color = session.lineColor;

    plot.x = domain::AxisSpec{.min = 0.0, .max = 20.0, .step = 1.0, .label = "steps"};

    plot.y = domain::AxisSpec{.min = 0.0, .max = 7.5, .step = 0.5, .label = "Y"};

    constexpr int pointCount = 21;
    plot.series.points.reserve(pointCount);

    int currentValue = 3;
    for (int xPos = 0; xPos < pointCount; ++xPos) {
        plot.series.points.push_back(
            domain::Point{.x = static_cast<double>(xPos), .y = static_cast<double>(currentValue)});

        if ((xPos % 2) == 0) {
            ++currentValue;
        } else {
            --currentValue;
        }

        if (currentValue < 0) {
            currentValue = 0;
        }
        if (currentValue > 7) {
            currentValue = 7;
        }
    }

    state.setTelemetryPlot(plot);
    return plot;
}

} // namespace application::useCases
