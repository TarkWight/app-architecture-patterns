#ifndef ICONFIGREPOSITORY_HPP
#define ICONFIGREPOSITORY_HPP

#include "../Dto/TelemetryConfig.hpp"

#include <string>

namespace application::ports {

class IConfigRepository {
public:
    virtual ~IConfigRepository() = default;

    virtual application::dto::TelemetryConfig
    loadTelemetryConfig(const std::string &path) = 0;
};

} // namespace application::ports

#endif // ICONFIGREPOSITORY_HPP