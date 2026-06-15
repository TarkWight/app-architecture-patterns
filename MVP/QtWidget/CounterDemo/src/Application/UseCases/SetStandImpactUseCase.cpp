#include "SetStandImpactUseCase.hpp"

#include "../../Domain/AxisId.hpp"
#include "../../Domain/StandCommandMapper.hpp"

#include <utility>

namespace application::useCases {

SetStandImpactUseCase::SetStandImpactUseCase(application::session::SessionState &state,
                                             application::ports::ITelemetryClient &telemetryClient)
    : state(state), telemetryClient(telemetryClient) {
}

void SetStandImpactUseCase::setTarget(domain::WindImpact profile) {
    state.setTargetStandImpact(std::move(profile));
}

void SetStandImpactUseCase::setApplied(domain::WindImpact profile) {
    const auto target = state.get().targetStandImpact;
    const auto elapsed = state.get().elapsed;

    state.setAppliedStandImpact(profile);
    state.appendControlTraceSample(domain::ControlTraceSample::manualCommand(elapsed, target, profile));
    sendAppliedImpact(profile);
}

void SetStandImpactUseCase::sendAppliedImpact(const domain::WindImpact &profile) {
    const auto mapping = domain::StandCommandMapper::map(profile, state.get().angleOfAttackOscillation);
    state.setAngleOfAttackOscillation(mapping.angleOfAttackState);
    telemetryClient.setAxisCommand(domain::axis0, mapping.commands.axis0);
    telemetryClient.setAxisCommand(domain::axis1, mapping.commands.axis1);
}

} // namespace application::useCases
