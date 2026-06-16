#include "ControlPlotBuilder.hpp"

#include "../../Domain/ControlTrace.hpp"
#include "../../Domain/TestTimeSource.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace application::services {

namespace {

domain::DurationMinutes determinePreviewDuration(const session::SessionStateData &stateData) {
    if (stateData.testTimeSource == domain::TestTimeSource::OperatorDefined) {
        return stateData.operatorTestDuration;
    }

    return stateData.estimatedTestDuration;
}

domain::DurationMinutes determineGridDuration(const session::SessionStateData &stateData) {
    if (!stateData.controlTrace.empty()) {
        const double lastTraceMinute = stateData.controlTrace.back().time.minutes();
        return domain::DurationMinutes::required(std::max(1, static_cast<int>(std::ceil(lastTraceMinute))));
    }

    return determinePreviewDuration(stateData);
}

double traceAxisStepSeconds(double maxSeconds) {
    if (maxSeconds <= 10.0) {
        return 1.0;
    }
    if (maxSeconds <= 60.0) {
        return 10.0;
    }
    if (maxSeconds <= 300.0) {
        return 30.0;
    }
    if (maxSeconds <= 900.0) {
        return 60.0;
    }
    if (maxSeconds <= 3600.0) {
        return 300.0;
    }

    return 600.0;
}

bool hasFormulaSeries(const application::dto::PlotModel &plot) {
    return !plot.series.points.empty();
}

void addFormulaSeries(application::dto::PlotModel &plot) {
    application::dto::NamedSeries formula{};
    formula.label = "Формула";
    formula.color = plot.color;
    formula.series = std::move(plot.series);
    plot.seriesList.clear();
    plot.seriesList.push_back(std::move(formula));
}

void addControlTraceSeries(application::dto::PlotModel &plot, const domain::ControlTrace &trace) {
    if (trace.empty()) {
        return;
    }

    const bool overlayFormula = hasFormulaSeries(plot);

    if (overlayFormula) {
        addFormulaSeries(plot);
    }

    application::dto::NamedSeries target{};
    target.label = "Цель";
    target.color = application::dto::RgbColor{220, 60, 50};

    application::dto::NamedSeries safeCommand{};
    safeCommand.label = "Безопасная команда";
    safeCommand.color = application::dto::RgbColor{40, 110, 210};

    target.series.points.reserve(trace.size());
    safeCommand.series.points.reserve(trace.size());

    for (const auto &sample : trace.samples()) {
        const double x = overlayFormula ? sample.time.minutes() : sample.time.seconds();
        target.series.points.push_back(application::dto::Point{.x = x, .y = sample.targetValue.beaufort.value()});
        safeCommand.series.points.push_back(
            application::dto::Point{.x = x, .y = sample.safeCommandValue.beaufort.value()});
    }

    const double markerTimeSeconds = trace.back().time.seconds();
    const double markerX = overlayFormula ? trace.back().time.minutes() : markerTimeSeconds;

    if (!overlayFormula) {
        const double maxSeconds = std::max(10.0, std::ceil(markerTimeSeconds));
        plot.x = application::dto::AxisSpec{
            .min = 0.0, .max = maxSeconds, .step = traceAxisStepSeconds(maxSeconds), .label = "seconds"};
    }

    plot.series.points.clear();
    plot.seriesList.push_back(std::move(target));
    plot.seriesList.push_back(std::move(safeCommand));

    plot.marker = application::dto::PlotMarker{.x = markerX, .label = "Сейчас", .visible = true};
}

} // namespace

application::dto::PlotModel ControlPlotBuilder::build(const session::SessionStateData &stateData,
                                                      const domain::WindControlProfile &profile) const {
    application::dto::PlotModel plot{};
    plot.title = "Control chart";
    plot.color = stateData.lineColor;

    const int durationMinutes =
        profile.duration.value() > 0 ? profile.duration.value() : determineGridDuration(stateData).value();
    plot.x = application::dto::AxisSpec{0.0, static_cast<double>(std::max(1, durationMinutes)), 1.0, "minutes"};
    plot.y = application::dto::AxisSpec{0.0, domain::maxOperationalBeaufort, 0.5, "Beaufort"};

    plot.series.points.reserve(profile.samples.size());
    for (const auto &sample : profile.samples) {
        plot.series.points.push_back(application::dto::Point{.x = sample.time.minutes(), .y = sample.beaufort.value()});
    }

    addControlTraceSeries(plot, stateData.controlTrace);
    return plot;
}

} // namespace application::services
