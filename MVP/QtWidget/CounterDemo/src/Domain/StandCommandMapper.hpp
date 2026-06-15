#ifndef STANDCOMMANDMAPPER_HPP
#define STANDCOMMANDMAPPER_HPP

#include "AngleOfAttackOscillationPolicy.hpp"
#include "AxisControlCommand.hpp"
#include "WindImpact.hpp"

namespace domain {

struct StandAxisCommands {
    AxisControlCommand axis0{};
    AxisControlCommand axis1{};
};

struct StandCommandMappingResult {
    StandAxisCommands commands{};
    AngleOfAttackOscillationState angleOfAttackState{};
};

class StandCommandMapper final {
  public:
    [[nodiscard]] static StandAxisCommands map(const WindImpact &impact) {
        return map(impact, AngleOfAttackOscillationState{}).commands;
    }

    [[nodiscard]] static StandCommandMappingResult map(const WindImpact &impact,
                                                       AngleOfAttackOscillationState angleOfAttackState) {
        const auto angleStep =
            AngleOfAttackOscillationPolicy::nextTarget(angleOfAttackState, impact.direction, impact.angleOfAttack);
        return StandCommandMappingResult{
            .commands = StandAxisCommands{.axis0 = mapAxis0(impact), .axis1 = mapAxis1(impact, angleStep.target)},
            .angleOfAttackState = angleStep.state};
    }

  private:
    [[nodiscard]] static AxisControlCommand mapAxis0(const WindImpact &impact) {
        const float torque = static_cast<float>(impact.beaufort.value()) * axis0TorqueMultiplier;
        return boundedAxisCommand(AxisControlCommand{.position = 0.0F,
                                                     .velocity = axisCommandVelocity,
                                                     .torque = torque,
                                                     .cmd1 = true,
                                                     .cmd2 = true,
                                                     .cmd3 = true,
                                                     .cmd4 = true});
    }

    [[nodiscard]] static AxisControlCommand mapAxis1(const WindImpact &impact, WindDirection targetPosition) {
        const float torque = static_cast<float>(impact.beaufort.value()) * axis1TorqueMultiplier;
        return boundedAxisCommand(AxisControlCommand{.position = static_cast<float>(targetPosition.degrees()),
                                                     .velocity = axisCommandVelocity,
                                                     .torque = torque,
                                                     .cmd1 = true,
                                                     .cmd2 = false,
                                                     .cmd3 = false,
                                                     .cmd4 = false});
    }
};

} // namespace domain

#endif // STANDCOMMANDMAPPER_HPP
