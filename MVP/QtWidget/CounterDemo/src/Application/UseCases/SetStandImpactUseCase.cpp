#include "SetStandImpactUseCase.hpp"

#include "../../Domain/AxisControlCommand.hpp"
#include "../../Domain/AxisId.hpp"

#include <utility>

namespace application::useCases {

SetStandImpactUseCase::SetStandImpactUseCase(application::session::SessionState &state,
                                             application::ports::ITelemetryClient &telemetryClient)
    : state(state), telemetryClient(telemetryClient) {
}

void SetStandImpactUseCase::setTarget(domain::WindProfile profile) {
    state.setTargetStandImpact(std::move(profile));
}

void SetStandImpactUseCase::setApplied(domain::WindProfile profile) {
    state.setAppliedStandImpact(profile);
    sendAppliedImpact(profile);
}

void SetStandImpactUseCase::sendAppliedImpact(const domain::WindProfile &profile) {
    telemetryClient.setAxisCommand(domain::axis0, domain::axis0WindCommand(profile));
    telemetryClient.setAxisCommand(domain::axis1, domain::axis1WindCommand(profile));
}

} // namespace application::useCases
