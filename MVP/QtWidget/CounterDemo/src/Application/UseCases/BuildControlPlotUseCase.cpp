#include "BuildControlPlotUseCase.hpp"

namespace application::useCases {

BuildControlPlotUseCase::BuildControlPlotUseCase(session::SessionState &state, const ports::IFunctionEngine &engine)
    : state(state), engine(engine), estimateTestDurationUseCase(state) {
}

application::dto::PlotModel BuildControlPlotUseCase::execute() {
    estimateTestDurationUseCase.executeForAutoCalculated();

    const auto &protocol = state.protocol();
    const auto &control = state.control();
    auto profile = profilePreviewService.build(protocol, control, engine);
    auto plot = plotBuilder.build(protocol, control, profile);

    state.setControlProfile(std::move(profile));
    state.setControlPlot(plot);
    return plot;
}

application::dto::PlotModel BuildControlPlotUseCase::refreshFromState() {
    const auto &protocol = state.protocol();
    const auto &control = state.control();

    auto plot = plotBuilder.build(protocol, control, control.controlProfile);

    state.setControlPlot(plot);
    return plot;
}

} // namespace application::useCases
