#include "UavSpecificationMapper.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace application::services {

namespace {

using ParameterMap = std::vector<std::pair<std::string, std::string>>;

std::string trim(std::string value) {
    const auto isNotSpace = [](unsigned char character) { return std::isspace(character) == 0; };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), isNotSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), isNotSpace).base(), value.end());
    return value;
}

ParameterMap toParameterMap(const std::vector<domain::TestProtocolParameter> &parameters) {
    ParameterMap result{};
    for (const auto &parameter : parameters) {
        if (!parameter.key.empty()) {
            result.emplace_back(parameter.key, parameter.value);
        }
    }
    return result;
}

std::string textValue(const ParameterMap &parameters, const std::string &key) {
    const auto found =
        std::ranges::find_if(parameters, [&key](const auto &parameter) { return parameter.first == key; });
    if (found == parameters.end()) {
        return {};
    }
    return trim(found->second);
}

double doubleValue(const ParameterMap &parameters, const std::string &key) {
    auto value = textValue(parameters, key);
    std::replace(value.begin(), value.end(), ',', '.');
    char *end = nullptr;
    const auto parsed = std::strtod(value.c_str(), &end);
    return end == value.c_str() ? 0.0 : parsed;
}

int intValue(const ParameterMap &parameters, const std::string &key) {
    return static_cast<int>(doubleValue(parameters, key));
}

bool hasPassportData(const ParameterMap &parameters) {
    return std::ranges::any_of(parameters, [](const auto &parameter) {
        auto value = trim(parameter.second);
        std::replace(value.begin(), value.end(), ',', '.');
        return !value.empty() && value != "0" && value != "0.0";
    });
}

} // namespace

std::optional<domain::UavSpecification> UavSpecificationMapper::map(const domain::TestProtocol &protocol) const {
    const auto parameters = toParameterMap(protocol.droneParameters);
    if (!hasPassportData(parameters)) {
        return std::nullopt;
    }

    domain::UavSpecification specification{};

    specification.frame.model = textValue(parameters, "uav_model");
    specification.frame.serialNumber = textValue(parameters, "serial_number");
    specification.frame.flightPayload = doubleValue(parameters, "flight_payload");
    specification.frame.dragRatioPercent = doubleValue(parameters, "drag_ratio_percent");
    specification.frame.equipmentCurrent = doubleValue(parameters, "equipment_current");
    specification.frame.operationRange = doubleValue(parameters, "operation_range");
    specification.frame.maxSpeed = doubleValue(parameters, "max_speed");
    specification.frame.maxAltitude = doubleValue(parameters, "max_altitude");
    specification.frame.maxFlightTime = doubleValue(parameters, "max_flight_time");
    specification.frame.communicationType = textValue(parameters, "communication_type");

    specification.propeller.model = textValue(parameters, "propeller_model");
    specification.propeller.configuration = textValue(parameters, "propeller_configuration");
    specification.propeller.diameter = doubleValue(parameters, "propeller_diameter");
    specification.propeller.power = doubleValue(parameters, "propeller_power");
    specification.propeller.maxRpm = intValue(parameters, "propeller_max_rpm");
    specification.propeller.bladeCount = intValue(parameters, "propeller_blade_count");
    specification.propeller.material = textValue(parameters, "propeller_material");

    specification.battery.model = textValue(parameters, "battery_model");
    specification.battery.type = textValue(parameters, "battery_type");
    specification.battery.capacityMah = intValue(parameters, "battery_capacity_mah");
    specification.battery.cellCount = intValue(parameters, "battery_cell_count");
    specification.battery.dischargeRateC = doubleValue(parameters, "battery_discharge_rate_c");
    specification.battery.weight = doubleValue(parameters, "battery_weight");
    specification.battery.cellVoltage = doubleValue(parameters, "battery_cell_voltage");

    specification.motor.model = textValue(parameters, "motor_model");
    specification.motor.shaftDiameter = doubleValue(parameters, "motor_shaft_diameter");
    specification.motor.count = intValue(parameters, "motor_count");
    specification.motor.manufacturer = textValue(parameters, "motor_manufacturer");
    specification.motor.kv = intValue(parameters, "motor_kv");
    specification.motor.maxThrustKg = doubleValue(parameters, "motor_max_thrust_kg");
    specification.motor.peakCurrentA = doubleValue(parameters, "motor_peak_current_a");
    specification.motor.weight = doubleValue(parameters, "motor_weight");

    specification.firmwareVersion = textValue(parameters, "firmware_version");
    return specification;
}

} // namespace application::services
