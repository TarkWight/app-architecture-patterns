#include "CalculateAndBuildControlPlotUseCase.hpp"

#include "../../Domain/TestModeStatePolicy.hpp"
#include "../../Domain/TestTimeSource.hpp"

namespace application::useCases {

CalculateAndBuildControlPlotUseCase::CalculateAndBuildControlPlotUseCase(
    application::session::SessionState &state, const application::ports::IFunctionEngine &engine,
    BuildControlPlotUseCase &buildControlPlotUseCase)
    : state(state), engine(engine), buildControlPlotUseCase(buildControlPlotUseCase),
      estimateTestDurationUseCase(state) {
}

void CalculateAndBuildControlPlotUseCase::execute() {
    const auto &protocol = state.protocol();
    if (!domain::TestModeStatePolicy::usesControlProfile(protocol.testProtocol.testMode)) {
        buildControlPlotUseCase.execute();
        return;
    }

    const auto candidate = analyzer.analyze(protocol, state.control(), engine, analysisDuration());
    const auto result = estimateTestDurationUseCase.executeForImpact(candidate.impact, candidate.usedFunction);
    if (!result.duration.has_value()) {
        return;
    }

    buildControlPlotUseCase.execute();
}

domain::DurationMinutes CalculateAndBuildControlPlotUseCase::analysisDuration() const {
    const auto &protocol = state.protocol();
    if (protocol.testTimeSource == domain::TestTimeSource::OperatorDefined) {
        return protocol.operatorTestDuration;
    }

    return domain::DurationMinutes::required(application::services::defaultControlFunctionAnalysisWindowMinutes);
}

} // namespace application::useCases
