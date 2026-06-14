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

constexpr auto pdfReportTemplate = R"([pdf_report]
title = ""
organization = ""
license_number = ""
address = ""
test_mode = ""
test_program = ""
operator_name = ""
comment = ""
conclusion = ""
result = ""

[pdf_report.drone_parameters]
uav_model = ""
serial_number = ""
payload_kg = 0
dr_percent = 0
equipment_current_a = 0
range_km = 0
max_speed_kmh = 0
max_height_m = 0
max_flight_time_min = 0
communication_type = ""
propeller_model = ""
configuration = ""
diameter = 0
power = 0
max_rpm = 0
blade_count = 0
material = ""
battery_model = ""
battery_type = ""
nominal_capacity_mah = 0
cell_count_s = 0
discharge_rate_c = 0
weight_kg = 0
cell_voltage_v = 0
motor_model = ""
shaft_diameter_mm = 0
motor_count = 0
manufacturer = ""
kv = 0
max_thrust_kg = 0
peak_current_a = 0
motor_weight_kg = 0
firmware_version = ""
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

void ConfigTemplateService::createTemplate(ConfigTemplateType type) const {
    const auto path = pathFor(type);
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
        return pdfReportTemplate;
    }

    return telemetryTemplate;
}

} // namespace infrastructure::configTemplates
