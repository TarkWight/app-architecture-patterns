#ifndef FORMULATEMPLATE_HPP
#define FORMULATEMPLATE_HPP

#include "TestProtocol.hpp"

#include <array>
#include <string_view>

namespace domain {

struct FormulaTemplate {
    std::string_view key{};
    std::string_view title{};
    std::string_view expression{};
};

constexpr std::array<FormulaTemplate, 4> formulaTemplates{{
    FormulaTemplate{.key = "calm", .title = "Штиль", .expression = "0"},
    FormulaTemplate{
        .key = "max_parameters", .title = "Проверка максимальных параметров", .expression = "60 * sin(0.0053 * x)"},
    FormulaTemplate{.key = "temporal_perspective",
                    .title = "Исследование временной перспективы",
                    .expression = "sin(x) * (6.9 * sin(10 * x))"},
    FormulaTemplate{.key = "attenuated_oscillation",
                    .title = "Затухающая осцилляция",
                    .expression = "15 * abs(x * sin(x) / (1 + x**2))"},
}};

constexpr FormulaTemplate formulaTemplateByKey(std::string_view key) {
    for (const auto &formulaTemplate : formulaTemplates) {
        if (formulaTemplate.key == key) {
            return formulaTemplate;
        }
    }

    return formulaTemplates.front();
}

constexpr std::string_view formulaTemplateKeyByExpression(std::string_view expression) {
    for (const auto &formulaTemplate : formulaTemplates) {
        if (formulaTemplate.expression == expression) {
            return formulaTemplate.key;
        }
    }

    return {};
}

constexpr FormulaTemplate formulaTemplateForTestProgram(TestProgram program) {
    switch (program) {
    case TestProgram::Custom:
        return FormulaTemplate{.key = "custom", .title = "Пользовательское", .expression = ""};
    case TestProgram::StabilityInIdealConditions:
        return formulaTemplateByKey("calm");
    case TestProgram::MaximumWindLoad:
        return formulaTemplateByKey("max_parameters");
    case TestProgram::WindLoadTemporalPerspective:
        return formulaTemplateByKey("temporal_perspective");
    case TestProgram::AttenuatedOscillation:
        return formulaTemplateByKey("attenuated_oscillation");
    }

    return formulaTemplateByKey("calm");
}

} // namespace domain

#endif // FORMULATEMPLATE_HPP
