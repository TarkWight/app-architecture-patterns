#include "AppliedStandImpactSender.hpp"

#include "UavSpecificationMapper.hpp"

#include "../../Domain/AxisId.hpp"
#include "../../Domain/StandCommandMapper.hpp"
#include "../../Domain/StandImpactCalculationContext.hpp"
#include "../../Domain/YawOscillationPolicy.hpp"

namespace application::services {

AppliedStandImpactSender::AppliedStandImpactSender(application::ports::ITelemetryClient &telemetryClient)
    : telemetryClient(telemetryClient) {
}

void AppliedStandImpactSender::send(const domain::WindImpact &impact, domain::ElapsedSeconds elapsed,
                                    const domain::TestProtocol &protocol) const {
    const auto uavSpecification = UavSpecificationMapper{}.map(protocol);
    const auto yawOffset = domain::YawOscillationPolicy::calculate(domain::StandImpactCalculationContext{
        .impact = impact, .elapsed = elapsed, .uavSpecification = uavSpecification});
    const auto commands = domain::StandCommandMapper::map(impact, yawOffset);
    telemetryClient.setAxisCommand(domain::axis0, commands.axis0);
    telemetryClient.setAxisCommand(domain::axis1, commands.axis1);
}

} // namespace application::services
