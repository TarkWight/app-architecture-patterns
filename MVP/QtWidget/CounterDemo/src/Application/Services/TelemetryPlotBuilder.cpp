#include "TelemetryPlotBuilder.hpp"

#include "../../Domain/AxisId.hpp"

#include <algorithm>
#include <utility>

namespace application::services {

application::dto::PlotModel TelemetryPlotBuilder::build(const session::SessionStateData &stateData) const {
    application::dto::PlotModel plot{};
    plot.title = "Telemetry";
    plot.x.label = "seconds";
    plot.y = application::dto::AxisSpec{.min = -180.0, .max = 360.0, .step = 45.0, .label = "degrees"};

    const double windowSeconds = std::max(1.0, stateData.telemetryWindowSeconds);
    const double endSeconds = std::max(windowSeconds, stateData.telemetryWindowEndSeconds.seconds());
    const double startSeconds = std::max(0.0, endSeconds - windowSeconds);

    plot.x = application::dto::AxisSpec{
        .min = startSeconds, .max = startSeconds + windowSeconds, .step = 10.0, .label = "seconds"};

    application::dto::NamedSeries axisY{};
    axisY.label = "Ось Y / тангаж";
    axisY.color = stateData.telemetryAxisYColor;

    application::dto::NamedSeries axisZ{};
    axisZ.label = "Ось Z / направление";
    axisZ.color = stateData.telemetryAxisZColor;

    if (!stateData.telemetryHistory.empty()) {
        const double baseTimestamp = stateData.telemetryHistory.front().timestampSeconds;

        for (const auto &sample : stateData.telemetryHistory) {
            const double x = sample.timestampSeconds - baseTimestamp;
            if (x < startSeconds || x > plot.x.max) {
                continue;
            }

            if (sample.axisId == domain::axis0 && stateData.telemetryAxisYVisible) {
                axisY.series.points.push_back(application::dto::Point{.x = x, .y = sample.position});
            } else if (sample.axisId == domain::axis1 && stateData.telemetryAxisZVisible) {
                axisZ.series.points.push_back(application::dto::Point{.x = x, .y = sample.position});
            }
        }
    }

    if (stateData.telemetryAxisYVisible) {
        plot.seriesList.push_back(std::move(axisY));
    }

    if (stateData.telemetryAxisZVisible) {
        plot.seriesList.push_back(std::move(axisZ));
    }

    return plot;
}

} // namespace application::services
