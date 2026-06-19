#include "ReadinessDiagnosticMessageBuilder.hpp"

#include <iomanip>
#include <sstream>

namespace application::services {
namespace {

std::string number(double value, int precision = 2) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

std::string summaryFor(application::session::ReadinessStatus status) {
    switch (status) {
    case application::session::ReadinessStatus::Ok:
        return "Расчёт готовности выполнен. Испытание допустимо.";
    case application::session::ReadinessStatus::Warning:
        return "Расчёт готовности выполнен. Есть предупреждения.";
    case application::session::ReadinessStatus::Dangerous:
        return "Испытание потенциально опасно.";
    case application::session::ReadinessStatus::Failed:
        return "Расчёт готовности невозможен. Испытание потенциально опасно.";
    case application::session::ReadinessStatus::Unknown:
        return "Расчёт готовности не выполнен.";
    }

    return "Расчёт готовности не выполнен.";
}

} // namespace

std::string ReadinessDiagnosticMessage::toDisplayText() const {
    if (details.empty()) {
        return summary;
    }

    std::string text = summary;
    for (const auto &detail : details) {
        text += "\n- " + detail;
    }
    return text;
}

ReadinessDiagnosticMessage ReadinessDiagnosticMessageBuilder::build(const session::ReadinessStateData &readiness) {
    ReadinessDiagnosticMessage message{.summary = summaryFor(readiness.status)};

    for (const auto &error : readiness.errors) {
        message.details.push_back(messageForDiagnostic(error.code, readiness.values));
    }

    for (const auto &warning : readiness.warnings) {
        message.details.push_back(messageForDiagnostic(warning.code, readiness.values));
    }

    return message;
}

std::string
ReadinessDiagnosticMessageBuilder::messageForDiagnostic(domain::TestDurationDiagnosticCode code,
                                                        const domain::TestDurationDiagnosticValues &values) {
    switch (code) {
    case domain::TestDurationDiagnosticCode::TotalMassMissing:
        return "Полная масса БАС не задана и не может быть рассчитана из полезной нагрузки, АКБ и двигателей. "
               "Ожидается масса больше 0 кг; без неё нельзя оценить требуемую тягу.";
    case domain::TestDurationDiagnosticCode::TotalMassEstimated:
        return "Полная масса БАС не задана явно. Использована оценка по полезной нагрузке, АКБ и двигателям: " +
               number(values.totalMassKg) + " кг; расчёт длительности менее точен.";
    case domain::TestDurationDiagnosticCode::BatteryCapacityMissing:
        return "Ёмкость АКБ отсутствует, пуста или равна 0 мА·ч. Ожидается значение больше 0; без него нельзя "
               "оценить запас энергии.";
    case domain::TestDurationDiagnosticCode::BatteryCellCountMissing:
        return "Число ячеек АКБ отсутствует, пусто или равно 0. Ожидается значение больше 0; без него нельзя "
               "рассчитать напряжение батареи.";
    case domain::TestDurationDiagnosticCode::BatteryCellVoltageMissing:
        return "Напряжение одной ячейки АКБ отсутствует, пусто или равно 0 В. Ожидается значение больше 0; без него "
               "нельзя рассчитать мощность.";
    case domain::TestDurationDiagnosticCode::MotorCountMissing:
        return "Количество двигателей отсутствует, пусто или равно 0. Ожидается значение больше 0; без него нельзя "
               "распределить нагрузку по двигателям.";
    case domain::TestDurationDiagnosticCode::MotorPeakCurrentMissing:
        return "Пиковый ток двигателя отсутствует, пуст или равен 0 А. Ожидается значение больше 0; без него нельзя "
               "проверить токовую нагрузку.";
    case domain::TestDurationDiagnosticCode::FrontalAreaFallbackUsed:
        return "Фронтальная площадь БАС не задана или некорректна. Использовано fallback-значение " +
               number(values.frontalAreaM2) + " м²; аэродинамическая нагрузка может быть оценена неточно.";
    case domain::TestDurationDiagnosticCode::DragCoefficientFallbackUsed:
        return "Коэффициент сопротивления БАС не задан или некорректен. Использовано fallback-значение " +
               number(values.dragCoefficient) + "; ветровая нагрузка может быть оценена неточно.";
    case domain::TestDurationDiagnosticCode::MotorMaxThrustMissing:
        return "Максимальная тяга двигателя не задана. Проверка запаса тяги пропущена; испытание может превысить "
               "возможности силовой установки.";
    case domain::TestDurationDiagnosticCode::RequiredThrustExceedsAvailable:
        return "Требуемая тяга " + number(values.requiredThrustKg) + " кг превышает доступную тягу " +
               number(values.availableThrustKg) + " кг. Стендовый сценарий может требовать режим, недоступный для БАС.";
    case domain::TestDurationDiagnosticCode::MotorPeakCurrentExceeded:
        return "Расчётный суммарный ток двигателей " + number(values.estimatedCurrentA) +
               " А превышает допустимый пиковый ток двигателей " + number(values.maxMotorCurrentA) +
               " А. Возможен перегрев или отказ силовой установки.";
    case domain::TestDurationDiagnosticCode::BatteryDischargeCurrentExceeded:
        return "Расчётный ток разряда " + number(values.estimatedCurrentA) + " А превышает допустимый ток АКБ " +
               number(values.maxBatteryCurrentA) + " А. Возможна перегрузка аккумулятора.";
    case domain::TestDurationDiagnosticCode::AngleOfAttackClampedByMinCos:
        return "Угол атаки приводит к слишком малому cos(angle). Применён защитный нижний предел коэффициента; "
               "режим может быть физически некорректен для расчёта.";
    }

    return "Неизвестная диагностика расчёта готовности.";
}

} // namespace application::services
