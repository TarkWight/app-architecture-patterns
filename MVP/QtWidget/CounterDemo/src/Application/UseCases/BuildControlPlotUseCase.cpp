#include "BuildControlPlotUseCase.hpp"

namespace application::useCases {

BuildControlPlotUseCase::BuildControlPlotUseCase(session::SessionState &state, const ports::IFunctionEngine &engine)
    : state(state), engine(engine) {
}

application::dto::PlotModel BuildControlPlotUseCase::execute() {
    const auto &stateData = state.get();
    auto profile = profilePreviewService.build(stateData, engine);
    auto plot = plotBuilder.build(stateData, profile);

    state.setControlProfile(std::move(profile));
    state.setControlPlot(plot);
    return plot;
}

application::dto::PlotModel BuildControlPlotUseCase::refreshFromState() {
    const auto &stateData = state.get();

    auto plot = plotBuilder.build(stateData, stateData.control.controlProfile);

    state.setControlPlot(plot);
    return plot;
}

} // namespace application::useCases
