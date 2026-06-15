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
    const auto commands = domain::StandCommandMapper::map(profile);
    telemetryClient.setAxisCommand(domain::axis0, commands.axis0);
    telemetryClient.setAxisCommand(domain::axis1, commands.axis1);
}

} // namespace application::useCases
