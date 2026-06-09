#ifndef TELEMETRYCONFIG_HPP
#define TELEMETRYCONFIG_HPP

#include "AxisEndpointConfig.hpp"

namespace application::dto {

struct TelemetryConfig {
    int pollIntervalMs{1000};

    AxisEndpointConfig axis0{};
    AxisEndpointConfig axis1{};
};

} // namespace application::dto

#endif // TELEMETRYCONFIG_HPP