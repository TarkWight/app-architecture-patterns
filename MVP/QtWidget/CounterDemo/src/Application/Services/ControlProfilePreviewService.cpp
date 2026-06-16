#include "ControlProfilePreviewService.hpp"

#include "../../Domain/ControlProfileTiming.hpp"
#include "../../Domain/WindControlProfileCalculator.hpp"

namespace application::services {

domain::WindControlProfile ControlProfilePreviewService::build(const session::ProtocolStateData &protocol,
                                                               const session::ControlStateData &control,
                                                               const ports::IFunctionEngine &engine) const {
    const auto timing =
        domain::determineControlProfileTiming(protocol.testProtocol.testMode, protocol.testTimeSource,
                                              protocol.estimatedTestDuration, protocol.operatorTestDuration);

    if (!timing.formulaEnabled) {
        return domain::WindControlProfile{};
    }

    return domain::buildWindControlProfile(timing.duration, [&engine, &control](double timeMinutes) {
        return engine.eval(control.functionExpression.value, timeMinutes);
    });
}

} // namespace application::services
