#include "SetStandImpactUseCase.hpp"

#include "../../Domain/AxisControlCommand.hpp"
#include "../../Domain/AxisId.hpp"

#include <cmath>
#include <utility>

namespace application::useCases {

namespace {

constexpr float commandVelocity = 0.06F;
constexpr float axis0TorqueMultiplier = 1.3F;
constexpr double activeEpsilon = 0.001;

domain::AxisControlCommand makeAxis0Command(const domain::WindProfile &profile) {
    const float torque = static_cast<float>(profile.beaufort.value()) * axis0TorqueMultiplier;
    const bool active = std::abs(profile.angleOfAttack.degrees()) > activeEpsilon || std::abs(torque) > activeEpsilon;
    if (!active) {
        return domain::stopAxisCommand();
    }

    return domain::sanitize(domain::AxisControlCommand{.position = static_cast<float>(profile.angleOfAttack.degrees()),
                                                       .velocity = commandVelocity,
                                                       .torque = torque,
                                                       .cmd1 = true,
                                                       .cmd2 = true,
                                                       .cmd3 = true,
                                                       .cmd4 = true});
}

domain::AxisControlCommand makeAxis1Command(const domain::WindProfile &profile) {
    const float torque = static_cast<float>(profile.beaufort.value());
    const bool active = std::abs(profile.direction.degrees()) > activeEpsilon || std::abs(torque) > activeEpsilon;
    if (!active) {
        return domain::stopAxisCommand();
    }

    return domain::sanitize(domain::AxisControlCommand{.position = static_cast<float>(profile.direction.degrees()),
                                                       .velocity = commandVelocity,
                                                       .torque = torque,
                                                       .cmd1 = true,
                                                       .cmd2 = false,
                                                       .cmd3 = false,
                                                       .cmd4 = false});
}

} // namespace

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
    telemetryClient.setAxisCommand(domain::axis0, makeAxis0Command(profile));
    telemetryClient.setAxisCommand(domain::axis1, makeAxis1Command(profile));
}

} // namespace application::useCases
