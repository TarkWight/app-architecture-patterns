#include "LoadPdfReportDefaultsUseCase.hpp"

#include <array>
#include <algorithm>
#include <utility>

namespace application::useCases {

namespace {

std::vector<domain::TestProtocolParameter> requiredDroneParameters() {
    return {
        {"uav_model", "Модель БПЛА", ""},
        {"serial_number", "Серийный номер", ""},
        {"flight_payload", "Полетная нагрузка", "0"},
        {"drag_ratio_percent", "DR (%)", "0"},
        {"equipment_current", "Ток оборудования", "0"},
        {"operation_range", "Дальность действия", "0"},
        {"max_speed", "Скорость Max.", "0"},
        {"max_altitude", "Высота Max.", "0"},
        {"max_flight_time", "Время полета max.", "0"},
        {"communication_type", "Тип связи", ""},
        {"propeller_model", "Модель винтов", ""},
        {"propeller_configuration", "Конфигурация", ""},
        {"propeller_diameter", "Диаметр", "0"},
        {"propeller_power", "Мощность", "0"},
        {"propeller_max_rpm", "Max об/мин", "0"},
        {"propeller_blade_count", "Количество лопастей", "0"},
        {"propeller_material", "Материал", ""},
        {"battery_model", "Аккумулятор", ""},
        {"battery_type", "Тип аккумулятора", ""},
        {"battery_capacity_mah", "Номинальная емкость (mAh)", "0"},
        {"battery_cell_count", "Число ячеек (S)", "0"},
        {"battery_discharge_rate_c", "Скорость разряда (C)", "0"},
        {"battery_weight", "Вес аккумулятора", "0"},
        {"battery_cell_voltage", "Напряжение 1й ячейки", "0"},
        {"motor_model", "Модель двигателей", ""},
        {"motor_shaft_diameter", "Диаметр вала", "0"},
        {"motor_count", "Количество двигателей", "0"},
        {"motor_manufacturer", "Производитель", ""},
        {"motor_kv", "KV", "0"},
        {"motor_max_thrust_kg", "Максимальная тяга (кг)", "0"},
        {"motor_peak_current_a", "Пиковый ток (A)", "0"},
        {"motor_weight", "Вес двигателя", "0"},
        {"firmware_version", "Версия прошивки", ""},
    };
}

std::vector<domain::TestProtocolParameter>
mergeWithRequiredParameters(const std::vector<domain::TestProtocolParameter> &loadedParameters) {
    auto parameters = requiredDroneParameters();

    for (const auto &loadedParameter : loadedParameters) {
        auto existing = std::find_if(parameters.begin(), parameters.end(), [&loadedParameter](const auto &parameter) {
            return (!loadedParameter.key.empty() && parameter.key == loadedParameter.key) ||
                   (!loadedParameter.label.empty() && parameter.label == loadedParameter.label);
        });

        if (existing == parameters.end()) {
            parameters.push_back(loadedParameter);
            continue;
        }

        existing->value = loadedParameter.value;
    }

    return parameters;
}

void applyConfig(application::session::SessionState &state, const application::dto::PdfReportConfig &config) {
    state.setTestProtocolTitle(config.title);

    const std::array<std::string, 8> lines{
        config.organization, config.licenseNumber, config.address, config.operatorName,
        config.comment,      config.conclusion,    config.result,  {}};

    for (std::size_t i = 0; i < lines.size(); ++i) {
        state.setTestProtocolLine(static_cast<int>(i), lines[i]);
    }

    state.setTestProtocolMode(config.testMode.empty() ? "manual" : config.testMode);
    state.setTestProtocolProgram(config.testProgram.empty() ? "test1" : config.testProgram);
    state.setTestProtocolDroneParameters(mergeWithRequiredParameters(config.droneParameters));
}

application::dto::PdfReportConfig emptyTemplateConfig() {
    application::dto::PdfReportConfig config{};
    config.testMode = "manual";
    config.testProgram = "test1";
    config.droneParameters = requiredDroneParameters();
    return config;
}

} // namespace

LoadPdfReportDefaultsUseCase::LoadPdfReportDefaultsUseCase(application::session::SessionState &state,
                                                           application::ports::IConfigRepository &configRepository)
    : state(state), configRepository(configRepository) {
}

void LoadPdfReportDefaultsUseCase::execute(const std::string &configPath) {
    const auto config = configRepository.loadPdfReportConfig(configPath);
    applyConfig(state, config);
}

void LoadPdfReportDefaultsUseCase::applyEmptyDefaults() {
    applyConfig(state, application::dto::PdfReportConfig{});
}

void LoadPdfReportDefaultsUseCase::saveEmptyTemplate(const std::string &configPath) {
    configRepository.savePdfReportTemplate(configPath, emptyTemplateConfig());
}

} // namespace application::useCases
