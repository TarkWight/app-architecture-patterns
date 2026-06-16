#include "SetStandImpactUseCase.hpp"

#include "../../Domain/AxisId.hpp"
#include "../../Domain/StandImpactCalculationContext.hpp"
#include "../../Domain/StandCommandMapper.hpp"
#include "../../Domain/YawOscillationPolicy.hpp"
#include "../Services/UavSpecificationMapper.hpp"

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
    const auto target = state.control().targetStandImpact;
    const auto elapsed = state.execution().elapsed;

    state.setAppliedStandImpact(profile);
    state.appendControlTraceSample(domain::ControlTraceSample::manualCommand(elapsed, target, profile));
    sendAppliedImpact(profile, elapsed);
}

void SetStandImpactUseCase::sendAppliedImpact(const domain::WindImpact &profile, domain::ElapsedSeconds elapsed) {
    const auto uavSpecification = application::services::UavSpecificationMapper{}.map(state.protocol().testProtocol);
    const auto yawOffset = domain::YawOscillationPolicy::calculate(domain::StandImpactCalculationContext{
        .impact = profile, .elapsed = elapsed, .uavSpecification = uavSpecification});
    const auto commands = domain::StandCommandMapper::map(profile, yawOffset);
    telemetryClient.setAxisCommand(domain::axis0, commands.axis0);
    telemetryClient.setAxisCommand(domain::axis1, commands.axis1);
}

} // namespace application::useCases
