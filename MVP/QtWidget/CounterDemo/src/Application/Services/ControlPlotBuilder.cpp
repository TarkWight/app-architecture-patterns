#include "ControlPlotBuilder.hpp"

#include "../../Domain/ControlTrace.hpp"
#include "../../Domain/TestTimeSource.hpp"
#include "../../Localization/PlotStrings.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace application::services {

namespace {

domain::DurationMinutes determinePreviewDuration(const session::ProtocolStateData &protocol) {
    if (protocol.testTimeSource == domain::TestTimeSource::OperatorDefined) {
        return protocol.operatorTestDuration;
    }

    return protocol.estimatedTestDuration;
}

domain::DurationMinutes determineGridDuration(const session::ProtocolStateData &protocol,
                                              const session::ControlStateData &control) {
    if (!control.controlTrace.empty()) {
        const double lastTraceMinute = control.controlTrace.back().time.minutes();
        return domain::DurationMinutes::required(std::max(1, static_cast<int>(std::ceil(lastTraceMinute))));
    }

    return determinePreviewDuration(protocol);
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
    formula.label = localization::plot::formulaSeries;
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
    target.label = localization::plot::targetSeries;
    target.color = application::dto::RgbColor{220, 60, 50};

    application::dto::NamedSeries safeCommand{};
    safeCommand.label = localization::plot::safeCommandSeries;
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
        plot.x = application::dto::AxisSpec{.min = 0.0,
                                            .max = maxSeconds,
                                            .step = traceAxisStepSeconds(maxSeconds),
                                            .label = localization::plot::secondsAxis};
    }

    plot.series.points.clear();
    plot.seriesList.push_back(std::move(target));
    plot.seriesList.push_back(std::move(safeCommand));

    if (overlayFormula) {
        plot.marker =
            application::dto::PlotMarker{.x = markerX, .label = localization::plot::currentMarker, .visible = true};
    }
}

} // namespace

application::dto::PlotModel ControlPlotBuilder::build(const session::ProtocolStateData &protocol,
                                                      const session::ControlStateData &control,
                                                      const domain::WindControlProfile &profile) const {
    application::dto::PlotModel plot{};
    plot.title = localization::plot::controlTitle;
    plot.color = control.lineColor;

    const int durationMinutes =
        profile.duration.value() > 0 ? profile.duration.value() : determineGridDuration(protocol, control).value();
    plot.x = application::dto::AxisSpec{0.0, static_cast<double>(std::max(1, durationMinutes)), 1.0,
                                        localization::plot::minutesAxis};
    plot.y = application::dto::AxisSpec{0.0, domain::maxOperationalBeaufort, 0.5, localization::plot::beaufortAxis};

    plot.series.points.reserve(profile.samples.size());
    for (const auto &sample : profile.samples) {
        plot.series.points.push_back(application::dto::Point{.x = sample.time.minutes(), .y = sample.beaufort.value()});
    }

    addControlTraceSeries(plot, control.controlTrace);
    return plot;
}

} // namespace application::services
