#include "TomlConfigRepository.hpp"

#include <toml++/toml.h>

namespace infrastructure::config {

namespace {

void assignString(const toml::table &table, const char *key, std::string &target) {
    if (auto value = table[key].value<std::string>()) {
        target = *value;
    }
}

} // namespace

application::dto::TelemetryConfig TomlConfigRepository::loadTelemetryConfig(const std::string &path) {

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

application::dto::PdfReportConfig TomlConfigRepository::loadPdfReportConfig(const std::string &path) {
    const auto table = toml::parse_file(path);

    application::dto::PdfReportConfig config{};
    const auto *report = table["pdf_report"].as_table();
    if (report == nullptr) {
        return config;
    }

    assignString(*report, "title", config.title);
    assignString(*report, "organization", config.organization);
    assignString(*report, "license_number", config.licenseNumber);
    assignString(*report, "address", config.address);
    assignString(*report, "test_type", config.testType);
    assignString(*report, "operator_name", config.operatorName);
    assignString(*report, "comment", config.comment);
    assignString(*report, "conclusion", config.conclusion);
    assignString(*report, "result", config.result);

    return config;
}

} // namespace infrastructure::config
