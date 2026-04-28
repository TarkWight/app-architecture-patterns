#ifndef AXISTELEMETRYSAMPLE_HPP
#define AXISTELEMETRYSAMPLE_HPP

#include "AxisId.hpp"

namespace domain {

struct AxisTelemetrySample {
    AxisId axisId{Axis0};

    double timestampSeconds{0.0};

    float position{0.0F};
    float setPosition{0.0F};

    float torque{0.0F};
    float setTorque{0.0F};

    float voltage{0.0F};
    float current{0.0F};

    bool valid{false};
};

} // namespace domain

#endif // AXISTELEMETRYSAMPLE_HPP