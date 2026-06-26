#ifndef READINESSSTRINGS_HPP
#define READINESSSTRINGS_HPP

#include <string>

namespace localization::readiness {

inline constexpr const char *summaryOk = "Расчёт готовности выполнен. Испытание допустимо.";
inline constexpr const char *summaryWarning = "Расчёт готовности выполнен. Есть предупреждения.";
inline constexpr const char *summaryDangerous = "Испытание потенциально опасно.";
inline constexpr const char *summaryFailed = "Расчёт готовности невозможен. Испытание потенциально опасно.";
inline constexpr const char *summaryUnknown = "Расчёт готовности не выполнен.";
inline constexpr const char *unknownDiagnostic = "Неизвестная диагностика расчёта готовности.";

inline std::string worstCaseScenarioImpact(const std::string &beaufort, const std::string &angleOfAttack) {
    return "Расчёт выполнен по худшему участку сценария: Бофорт " + beaufort + ", угол атаки " + angleOfAttack + "°.";
}

inline std::string safeBeaufortLimit(const std::string &beaufort) {
    return "Безопасный предел по Бофорту: " + beaufort + ".";
}

inline std::string safeAngleOfAttackLimit(const std::string &angleOfAttack) {
    return "Безопасный предел угла атаки: ±" + angleOfAttack + "°.";
}

inline constexpr const char *safeLimitsUnavailable =
    "Безопасные пределы воздействия недоступны: не хватает критичных паспортных данных БАС.";

inline constexpr const char *totalMassMissing =
    "Полная масса БАС не задана и не может быть рассчитана из полезной нагрузки, АКБ и двигателей. Ожидается масса "
    "больше 0 кг; без неё нельзя оценить требуемую тягу.";

inline std::string totalMassEstimated(const std::string &totalMassKg) {
    return "Полная масса БАС не задана явно. Использована оценка по полезной нагрузке, АКБ и двигателям: " +
           totalMassKg + " кг; расчёт длительности менее точен.";
}

inline constexpr const char *batteryCapacityMissing =
    "Ёмкость АКБ отсутствует, пуста или равна 0 мА·ч. Ожидается значение больше 0; без него нельзя оценить запас "
    "энергии.";
inline constexpr const char *batteryCellCountMissing =
    "Число ячеек АКБ отсутствует, пусто или равно 0. Ожидается значение больше 0; без него нельзя рассчитать "
    "напряжение батареи.";
inline constexpr const char *batteryCellVoltageMissing =
    "Напряжение одной ячейки АКБ отсутствует, пусто или равно 0 В. Ожидается значение больше 0; без него нельзя "
    "рассчитать мощность.";
inline constexpr const char *motorCountMissing =
    "Количество двигателей отсутствует, пусто или равно 0. Ожидается значение больше 0; без него нельзя распределить "
    "нагрузку по двигателям.";
inline constexpr const char *motorPeakCurrentMissing =
    "Пиковый ток двигателя отсутствует, пуст или равен 0 А. Ожидается значение больше 0; без него нельзя проверить "
    "токовую нагрузку.";

inline std::string frontalAreaFallbackUsed(const std::string &frontalAreaM2) {
    return "Фронтальная площадь БАС не задана или некорректна. Использовано резервное значение " + frontalAreaM2 +
           " м²; аэродинамическая нагрузка может быть оценена неточно.";
}

inline std::string dragCoefficientFallbackUsed(const std::string &dragCoefficient) {
    return "Коэффициент сопротивления БАС не задан или некорректен. Использовано резервное значение " +
           dragCoefficient + "; ветровая нагрузка может быть оценена неточно.";
}

inline constexpr const char *motorMaxThrustMissing =
    "Максимальная тяга двигателя не задана. Проверка запаса тяги пропущена; испытание может превысить возможности "
    "силовой установки.";

inline std::string requiredThrustExceedsAvailable(const std::string &requiredThrustKg,
                                                  const std::string &availableThrustKg) {
    return "Требуемая тяга " + requiredThrustKg + " кг превышает доступную тягу " + availableThrustKg +
           " кг. Стендовый сценарий может требовать режим, недоступный для БАС.";
}

inline std::string motorPeakCurrentExceeded(const std::string &estimatedCurrentA, const std::string &maxMotorCurrentA) {
    return "Расчётный суммарный ток двигателей " + estimatedCurrentA +
           " А превышает допустимый пиковый ток двигателей " + maxMotorCurrentA +
           " А. Возможен перегрев или отказ силовой установки.";
}

inline std::string batteryDischargeCurrentExceeded(const std::string &estimatedCurrentA,
                                                   const std::string &maxBatteryCurrentA) {
    return "Расчётный ток разряда " + estimatedCurrentA + " А превышает допустимый ток АКБ " + maxBatteryCurrentA +
           " А. Возможна перегрузка аккумулятора.";
}

inline constexpr const char *angleOfAttackClampedByMinCos =
    "Угол атаки приводит к слишком малому косинусу угла. Применён защитный нижний предел коэффициента; режим может "
    "быть физически некорректен для расчёта.";

} // namespace localization::readiness

#endif // READINESSSTRINGS_HPP
