#include "TelemetryPlotBuilder.hpp"

#include "../../Domain/AxisId.hpp"

#include <algorithm>
#include <utility>

namespace application::services {

application::dto::PlotModel TelemetryPlotBuilder::build(const session::TelemetryStateData &telemetry) const {
    application::dto::PlotModel plot{};
    plot.title = "Telemetry";
    plot.x.label = "seconds";
    plot.y = application::dto::AxisSpec{.min = 0.0, .max = 360.0, .step = 45.0, .label = "degrees"};

    const double windowSeconds = std::max(1.0, telemetry.telemetryWindowSeconds);
    const double endSeconds = std::max(windowSeconds, telemetry.telemetryWindowEndSeconds.seconds());
    const double startSeconds = std::max(0.0, endSeconds - windowSeconds);

    plot.x = application::dto::AxisSpec{.min = startSeconds,
                                        .max = startSeconds + windowSeconds,
                                        .step = 10.0,
                                        .label = "seconds",
                                        .labelPrecision = 0};

    application::dto::NamedSeries axisY{};
    axisY.label = "Ось Y / тангаж";
    axisY.color = telemetry.telemetryAxisYColor;

    application::dto::NamedSeries axisZ{};
    axisZ.label = "Ось Z / направление";
    axisZ.color = telemetry.telemetryAxisZColor;

    if (!telemetry.telemetryHistory.empty()) {
        const double baseTimestamp = telemetry.telemetryHistory.front().timestampSeconds;

        for (const auto &sample : telemetry.telemetryHistory) {
            const double x = sample.timestampSeconds - baseTimestamp;
            if (x < startSeconds || x > plot.x.max) {
                continue;
            }

            if (sample.axisId == domain::axis0 && telemetry.telemetryAxisYVisible) {
                axisY.series.points.push_back(application::dto::Point{.x = x, .y = sample.position});
            } else if (sample.axisId == domain::axis1 && telemetry.telemetryAxisZVisible) {
                axisZ.series.points.push_back(application::dto::Point{.x = x, .y = sample.position});
            }
        }
    }

    if (telemetry.telemetryAxisYVisible) {
        plot.seriesList.push_back(std::move(axisY));
    }

    if (telemetry.telemetryAxisZVisible) {
        plot.seriesList.push_back(std::move(axisZ));
    }

    return plot;
}

} // namespace application::services
