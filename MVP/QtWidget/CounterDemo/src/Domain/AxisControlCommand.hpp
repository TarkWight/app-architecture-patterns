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

inline AxisControlCommand stopAxisCommand() {
    return AxisControlCommand{};
}

inline AxisControlCommand sanitize(AxisControlCommand command) {
    command.velocity = std::clamp(command.velocity, -1.0F, 1.0F);
    command.torque = std::clamp(command.torque, 0.0F, 50.0F);
    return command;
}

} // namespace domain

#endif // AXISCONTROLCOMMAND_HPP
