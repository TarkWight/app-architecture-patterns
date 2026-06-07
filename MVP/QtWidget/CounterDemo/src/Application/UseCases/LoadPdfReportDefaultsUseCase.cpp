#include "LoadPdfReportDefaultsUseCase.hpp"

#include <array>
#include <algorithm>
#include <utility>

namespace application::useCases {

namespace {

std::vector<domain::TestProtocolParameter> requiredDroneParameters() {
    return {
        {"Модель БПЛА", ""},
        {"Серийный номер", ""},
        {"Полетная нагрузка", "0"},
        {"DR (%)", "0"},
        {"Ток оборудования", "0"},
        {"Дальность действия", "0"},
        {"Скорость Max.", "0"},
        {"Высота Max.", "0"},
        {"Время полета max.", "0"},
        {"Тип связи", ""},
        {"Модель винтов", ""},
        {"Конфигурация", ""},
        {"Диаметр", "0"},
        {"Мощность", "0"},
        {"Max об/мин", "0"},
        {"Количество лопастей", "0"},
        {"Материал", ""},
        {"Аккумулятор", ""},
        {"Тип аккумулятора", ""},
        {"Номинальная емкость (mAh)", "0"},
        {"Число ячеек (S)", "0"},
        {"Скорость разряда (C)", "0"},
        {"Вес аккумулятора", "0"},
        {"Напряжение 1й ячейки", "0"},
        {"Модель двигателей", ""},
        {"Диаметр вала", "0"},
        {"Количество двигателей", "0"},
        {"Производитель", ""},
        {"KV", "0"},
        {"Максимальная тяга (кг)", "0"},
        {"Пиковый ток (A)", "0"},
        {"Вес двигателя", "0"},
        {"Версия прошивки", ""},
    };
}

std::vector<domain::TestProtocolParameter>
mergeWithRequiredParameters(const std::vector<domain::TestProtocolParameter> &loadedParameters) {
    auto parameters = requiredDroneParameters();

    for (const auto &loadedParameter : loadedParameters) {
        auto existing = std::find_if(parameters.begin(), parameters.end(), [&loadedParameter](const auto &parameter) {
            return parameter.label == loadedParameter.label;
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

} // namespace application::useCases
