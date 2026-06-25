#include "SetStandImpactUseCase.hpp"

#include <utility>

namespace application::useCases {

SetStandImpactUseCase::SetStandImpactUseCase(application::session::SessionState &state,
                                             application::ports::ITelemetryClient &telemetryClient)
    : state(state), appliedStandImpactSender(telemetryClient) {
}

void SetStandImpactUseCase::setTarget(domain::WindImpact profile) {
    state.setTargetStandImpact(std::move(profile));
}

void SetStandImpactUseCase::setApplied(domain::WindImpact profile) {
    const auto target = state.control().targetStandImpact;
    const auto elapsed = state.execution().elapsed;

    state.setAppliedStandImpact(profile);
    state.appendControlTraceSample(domain::ControlTraceSample::manualCommand(elapsed, target, profile));
    appliedStandImpactSender.send(profile, elapsed, state.protocol().testProtocol,
                                  state.control().useAngleOfAttackModel);
}

} // namespace application::useCases
