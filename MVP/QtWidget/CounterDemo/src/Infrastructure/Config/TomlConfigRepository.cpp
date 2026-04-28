#include "TomlConfigRepository.hpp"

#include <toml++/toml.h>

namespace infrastructure::config {

application::dto::TelemetryConfig
TomlConfigRepository::loadTelemetryConfig(const std::string &path) {

    const auto table = toml::parse_file(path);

    application::dto::TelemetryConfig config{};

    if (auto value = table["telemetry"]["poll_interval_ms"].value<int>()) {
        config.pollIntervalMs = *value;
    }

    if (auto axis0 = table["telemetry"]["axis0"].as_table()) {
        if (auto value = (*axis0)["host"].value<std::string>())
            config.axis0.host = *value;

        if (auto value = (*axis0)["port"].value<int>())
            config.axis0.port = *value;

        if (auto value = (*axis0)["enabled"].value<bool>())
            config.axis0.enabled = *value;
    }

    if (auto axis1 = table["telemetry"]["axis1"].as_table()) {
        if (auto value = (*axis1)["host"].value<std::string>())
            config.axis1.host = *value;

        if (auto value = (*axis1)["port"].value<int>())
            config.axis1.port = *value;

        if (auto value = (*axis1)["enabled"].value<bool>())
            config.axis1.enabled = *value;
    }

    return config;
}

} // namespace infrastructure::config