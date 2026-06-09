#ifndef AXISENDPOINTCONFIG_HPP
#define AXISENDPOINTCONFIG_HPP

#include <string>

namespace application::dto {

struct AxisEndpointConfig {
    std::string host{};
    int port{0};
    bool enabled{true};
};

} // namespace application::dto

#endif // AXISENDPOINTCONFIG_HPP