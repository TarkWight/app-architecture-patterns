#include "BuildControlPlotUseCase.hpp"

#include "../../Domain/ControlProfileTiming.hpp"
#include "../../Domain/WindControlProfile.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>

namespace application::useCases {

namespace {

domain::WindControlProfile buildProfile(const session::SessionStateData &stateData,
                                        const ports::IFunctionEngine &engine) {
    domain::WindControlProfile profile{};
    profile.sampleIntervalSeconds = domain::windControlProfileSampleIntervalSeconds;

    const auto timing =
        domain::determineControlProfileTiming(stateData.testProtocol.testMode, stateData.testTimeSource,
                                              stateData.estimatedTestDuration, stateData.operatorTestDuration);
    if (!timing.formulaEnabled) {
        return profile;
    }

    profile.durationMinutes = timing.duration.value();
    const int sampleCount =
        static_cast<int>(static_cast<double>(profile.durationMinutes * 60) / profile.sampleIntervalSeconds);
    profile.samples.reserve(static_cast<std::size_t>(sampleCount));

    for (int index = 0; index < sampleCount; ++index) {
        const double timeSeconds = static_cast<double>(index) * profile.sampleIntervalSeconds;
        const double timeMinutes = timeSeconds / 60.0;
        const double rawBeaufort = engine.eval(stateData.functionExpression.value, timeMinutes);

        profile.samples.push_back(domain::WindControlSample{
            .timeSeconds = timeSeconds,
            .timeMinutes = timeMinutes,
            .beaufort = domain::Beaufort::from(rawBeaufort),
        });
    }

    return profile;
}

domain::PlotModel buildPlot(const session::SessionStateData &stateData, const domain::WindControlProfile &profile) {
    domain::PlotModel plot{};
    plot.title = profile.samples.empty() ? "Manual control" : "Control chart";
    plot.color = stateData.lineColor;
    plot.x = domain::AxisSpec{0.0, static_cast<double>(std::max(1, profile.durationMinutes)), 1.0, "minutes"};
    plot.y = domain::AxisSpec{0.0, domain::maxOperationalBeaufort, 0.5, "Beaufort"};

    plot.series.points.reserve(profile.samples.size());
    for (const auto &sample : profile.samples) {
        plot.series.points.push_back(domain::Point{.x = sample.timeMinutes, .y = sample.beaufort.value()});
    }

    return plot;
}

} // namespace

BuildControlPlotUseCase::BuildControlPlotUseCase(session::SessionState &state, const ports::IFunctionEngine &engine)
    : state(state), engine(engine) {
}

domain::PlotModel BuildControlPlotUseCase::execute() {
    const auto &stateData = state.get();

    auto profile = buildProfile(stateData, engine);
    auto plot = buildPlot(stateData, profile);

    state.setControlProfile(std::move(profile));
    state.setControlPlot(plot);
    return plot;
}

} // namespace application::useCases
