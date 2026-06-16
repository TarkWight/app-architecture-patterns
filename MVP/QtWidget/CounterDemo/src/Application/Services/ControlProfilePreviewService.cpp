#include "ControlProfilePreviewService.hpp"

#include "../../Domain/ControlProfileTiming.hpp"
#include "../../Domain/WindControlProfileCalculator.hpp"

namespace application::services {

domain::WindControlProfile ControlProfilePreviewService::build(const session::SessionStateData &stateData,
                                                               const ports::IFunctionEngine &engine) const {
    const auto timing = domain::determineControlProfileTiming(
        stateData.protocol.testProtocol.testMode, stateData.protocol.testTimeSource,
        stateData.protocol.estimatedTestDuration, stateData.protocol.operatorTestDuration);

    if (!timing.formulaEnabled) {
        return domain::WindControlProfile{};
    }

    return domain::buildWindControlProfile(timing.duration, [&engine, &stateData](double timeMinutes) {
        return engine.eval(stateData.control.functionExpression.value, timeMinutes);
    });
}

} // namespace application::services
