#include "BuildControlPlotUseCase.hpp"

#include "../../Domain/ControlProfileTiming.hpp"
#include "../../Domain/WindControlProfile.hpp"
#include "../../Domain/WindControlProfileCalculator.hpp"

namespace application::useCases {

BuildControlPlotUseCase::BuildControlPlotUseCase(session::SessionState &state, const ports::IFunctionEngine &engine)
    : state(state), engine(engine) {
}

application::dto::PlotModel BuildControlPlotUseCase::execute() {
    const auto &stateData = state.get();
    const auto timing =
        domain::determineControlProfileTiming(stateData.testProtocol.testMode, stateData.testTimeSource,
                                              stateData.estimatedTestDuration, stateData.operatorTestDuration);

    auto profile =
        timing.formulaEnabled
            ? domain::buildWindControlProfile(timing.duration,
                                              [this, &stateData](double timeMinutes) {
                                                  return engine.eval(stateData.functionExpression.value, timeMinutes);
                                              })
            : domain::WindControlProfile{};
    auto plot = plotBuilder.build(stateData, profile);

    state.setControlProfile(std::move(profile));
    state.setControlPlot(plot);
    return plot;
}

application::dto::PlotModel BuildControlPlotUseCase::refreshFromState() {
    const auto &stateData = state.get();

    auto plot = plotBuilder.build(stateData, stateData.controlProfile);

    state.setControlPlot(plot);
    return plot;
}

} // namespace application::useCases
