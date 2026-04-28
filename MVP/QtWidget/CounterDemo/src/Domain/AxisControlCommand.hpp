#ifndef AXISCONTROLCOMMAND_HPP
#define AXISCONTROLCOMMAND_HPP

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

} // namespace domain

#endif // AXISCONTROLCOMMAND_HPP