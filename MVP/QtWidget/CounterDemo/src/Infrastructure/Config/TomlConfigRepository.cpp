#include "TomlConfigRepository.hpp"

#include <toml++/toml.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace infrastructure::config {

namespace {

void assignString(const toml::table &table, const char *key, std::string &target) {
    if (auto value = table[key].value<std::string>()) {
        target = *value;
    }
}

std::string nodeToString(const toml::node &node) {
    if (auto value = node.value<std::string>()) {
        return *value;
    }

    if (auto value = node.value<int64_t>()) {
        return std::to_string(*value);
    }

    if (auto value = node.value<double>()) {
        std::ostringstream stream;
        stream << *value;
        return stream.str();
    }

    if (auto value = node.value<bool>()) {
        return *value ? "true" : "false";
    }

    return {};
}

std::string escapeTomlString(const std::string &value) {
    std::string escaped;
    escaped.reserve(value.size());

    for (const char character : value) {
        if (character == '\\' || character == '"') {
            escaped.push_back('\\');
        }

        escaped.push_back(character);
    }

    return escaped;
}

bool isZeroValue(const std::string &value) {
    return value == "0";
}

void writeTomlString(std::ostream &stream, const char *key, const std::string &value) {
    stream << key << " = \"" << escapeTomlString(value) << "\"\n";
}

void writeTomlParameter(std::ostream &stream, const domain::TestProtocolParameter &parameter) {
    stream << "# " << parameter.label << "\n";
    stream << parameter.key << " = ";

    if (isZeroValue(parameter.value)) {
        stream << "0\n";
        return;
    }

    stream << '"' << escapeTomlString(parameter.value) << "\"\n";
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
    assignString(*report, "test_mode", config.testMode);
    assignString(*report, "test_program", config.testProgram);
    assignString(*report, "operator_name", config.operatorName);
    assignString(*report, "comment", config.comment);
    assignString(*report, "conclusion", config.conclusion);
    assignString(*report, "result", config.result);

    if (const auto *parametersTable = (*report)["drone_parameters"].as_table()) {
        for (const auto &[key, valueNode] : *parametersTable) {
            domain::TestProtocolParameter field{};
            field.key = std::string{key.str()};
            field.value = nodeToString(valueNode);
            config.droneParameters.push_back(std::move(field));
        }
    } else if (const auto *parameters = (*report)["drone_parameters"].as_array()) {
        for (const auto &parameterNode : *parameters) {
            const auto *parameter = parameterNode.as_table();
            if (parameter == nullptr) {
                continue;
            }

            domain::TestProtocolParameter field{};
            assignString(*parameter, "key", field.key);
            assignString(*parameter, "label", field.label);
            assignString(*parameter, "value", field.value);

            if (!field.key.empty() || !field.label.empty()) {
                config.droneParameters.push_back(std::move(field));
            }
        }
    }

    return config;
}

void TomlConfigRepository::savePdfReportTemplate(const std::string &path,
                                                 const application::dto::PdfReportConfig &config) {
    std::ofstream stream{path};
    if (!stream.is_open()) {
        throw std::runtime_error{"cannot open PDF report TOML template for writing"};
    }

    stream << "[pdf_report]\n";
    writeTomlString(stream, "title", config.title);
    writeTomlString(stream, "organization", config.organization);
    writeTomlString(stream, "license_number", config.licenseNumber);
    writeTomlString(stream, "address", config.address);
    writeTomlString(stream, "test_mode", config.testMode);
    writeTomlString(stream, "test_program", config.testProgram);
    writeTomlString(stream, "operator_name", config.operatorName);
    writeTomlString(stream, "comment", config.comment);
    writeTomlString(stream, "conclusion", config.conclusion);
    writeTomlString(stream, "result", config.result);

    stream << "\n[pdf_report.drone_parameters]\n";
    for (const auto &parameter : config.droneParameters) {
        if (parameter.key.empty()) {
            continue;
        }

        writeTomlParameter(stream, parameter);
    }
}

} // namespace infrastructure::config
