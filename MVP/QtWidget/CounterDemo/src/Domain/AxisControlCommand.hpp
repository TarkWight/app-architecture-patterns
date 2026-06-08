#ifndef AXISCONTROLCOMMAND_HPP
#define AXISCONTROLCOMMAND_HPP

#include "WindImpact.hpp"

#include <algorithm>
#include <cmath>

namespace domain {

struct AxisControlCommand {
    float position{0.0F};
    float velocity{0.0F};
    float torque{0.0F};

    bool cmd1{false};
    bool cmd2{false};
    bool cmd3{false};
    bool cmd4{false};
};

constexpr float axisCommandVelocity = 0.06F;
constexpr float axis0TorqueMultiplier = 1.3F;
constexpr double axisCommandActiveEpsilon = 0.001;

inline AxisControlCommand stopAxisCommand() {
    return AxisControlCommand{};
}

inline AxisControlCommand boundedAxisCommand(AxisControlCommand command) {
    command.velocity = std::clamp(command.velocity, -1.0F, 1.0F);
    command.torque = std::clamp(command.torque, 0.0F, 50.0F);
    return command;
}

inline AxisControlCommand axis0WindCommand(const WindImpact &profile) {
    const float torque = static_cast<float>(profile.beaufort.value()) * axis0TorqueMultiplier;
    const bool active = std::abs(profile.angleOfAttack.degrees()) > axisCommandActiveEpsilon ||
                        std::abs(torque) > axisCommandActiveEpsilon;
    if (!active) {
        return stopAxisCommand();
    }

    return boundedAxisCommand(AxisControlCommand{.position = static_cast<float>(profile.angleOfAttack.degrees()),
                                                 .velocity = axisCommandVelocity,
                                                 .torque = torque,
                                                 .cmd1 = true,
                                                 .cmd2 = true,
                                                 .cmd3 = true,
                                                 .cmd4 = true});
}

inline AxisControlCommand axis1WindCommand(const WindImpact &profile) {
    const float torque = static_cast<float>(profile.beaufort.value());
    const bool active =
        std::abs(profile.direction.degrees()) > axisCommandActiveEpsilon || std::abs(torque) > axisCommandActiveEpsilon;
    if (!active) {
        return stopAxisCommand();
    }

    return boundedAxisCommand(AxisControlCommand{.position = static_cast<float>(profile.direction.degrees()),
                                                 .velocity = axisCommandVelocity,
                                                 .torque = torque,
                                                 .cmd1 = true,
                                                 .cmd2 = false,
                                                 .cmd3 = false,
                                                 .cmd4 = false});
}

} // namespace domain

#endif // AXISCONTROLCOMMAND_HPP
