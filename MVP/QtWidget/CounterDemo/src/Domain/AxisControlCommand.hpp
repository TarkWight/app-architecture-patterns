#ifndef AXISCONTROLCOMMAND_HPP
#define AXISCONTROLCOMMAND_HPP

#include <algorithm>

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
constexpr float axis0TorqueMultiplier = 6.5F;
constexpr float axis1TorqueMultiplier = 5.0F;

inline AxisControlCommand stopAxisCommand() {
    return AxisControlCommand{};
}

inline AxisControlCommand boundedAxisCommand(AxisControlCommand command) {
    command.velocity = std::clamp(command.velocity, -1.0F, 1.0F);
    command.torque = std::clamp(command.torque, 0.0F, 50.0F);
    return command;
}

} // namespace domain

#endif // AXISCONTROLCOMMAND_HPP
