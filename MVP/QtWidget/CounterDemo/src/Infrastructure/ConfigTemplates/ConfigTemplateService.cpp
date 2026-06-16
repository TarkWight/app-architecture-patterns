#include "ConfigTemplateService.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace infrastructure::configTemplates {

namespace {

constexpr auto telemetryTemplate = R"([telemetry]
poll_interval_ms = 1000

[telemetry.axis0]
enabled = true
host = "192.168.1.100"
port = 11540

[telemetry.axis1]
enabled = true
host = "192.168.1.101"
port = 11520
)";

} // namespace

ConfigTemplateService::ConfigTemplateService(const IAppFileLocationProvider &locationProvider)
    : locationProvider(locationProvider) {
}

std::filesystem::path ConfigTemplateService::pathFor(ConfigTemplateType type) const {
    return locationProvider.applicationDir() / fileNameFor(type);
}

bool ConfigTemplateService::exists(ConfigTemplateType type) const {
    return std::filesystem::exists(pathFor(type));
}

ConfigTemplateResolution ConfigTemplateService::resolvePath(ConfigTemplateType type,
                                                            const std::filesystem::path &operatorSelectedPath) const {
    const bool operatorPathProvided = !operatorSelectedPath.empty();
    const auto path = operatorPathProvided ? operatorSelectedPath : pathFor(type);

    if (std::filesystem::exists(path)) {
        return ConfigTemplateResolution{.type = type, .path = path, .status = ConfigTemplateResolutionStatus::Found};
    }

    return ConfigTemplateResolution{.type = type,
                                    .path = path,
                                    .status = operatorPathProvided
                                                  ? ConfigTemplateResolutionStatus::MissingOperatorSelected
                                                  : ConfigTemplateResolutionStatus::MissingDefault};
}

std::vector<ConfigTemplateState> ConfigTemplateService::inspectAll() const {
    return std::vector<ConfigTemplateState>{
        ConfigTemplateState{.type = ConfigTemplateType::Telemetry,
                            .path = pathFor(ConfigTemplateType::Telemetry),
                            .exists = exists(ConfigTemplateType::Telemetry)},
        ConfigTemplateState{.type = ConfigTemplateType::PdfReport,
                            .path = pathFor(ConfigTemplateType::PdfReport),
                            .exists = exists(ConfigTemplateType::PdfReport)},
    };
}

void ConfigTemplateService::createTemplate(ConfigTemplateType type, const std::filesystem::path &path) const {
    if (std::filesystem::exists(path)) {
        return;
    }

    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream stream{path};
    if (!stream.is_open()) {
        throw std::runtime_error{"cannot create config template: " + path.string()};
    }

    stream << templateContentFor(type);
}

const char *ConfigTemplateService::fileNameFor(ConfigTemplateType type) {
    switch (type) {
    case ConfigTemplateType::Telemetry:
        return "telemetry.toml";
    case ConfigTemplateType::PdfReport:
        return "pdf_report.toml";
    }

    return "telemetry.toml";
}

const char *ConfigTemplateService::templateContentFor(ConfigTemplateType type) {
    switch (type) {
    case ConfigTemplateType::Telemetry:
        return telemetryTemplate;
    case ConfigTemplateType::PdfReport:
        throw std::logic_error{"PDF report template is created by TestProtocolTab"};
    }

    return telemetryTemplate;
}

} // namespace infrastructure::configTemplates
