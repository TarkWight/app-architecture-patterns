#ifndef CONTROLCHARTSSTRINGS_HPP
#define CONTROLCHARTSSTRINGS_HPP

#include <string>

namespace localization::controlCharts {

inline constexpr const char *minutesUpdated = "Время теста обновлено";
inline constexpr const char *testModeUpdated = "Тип испытания обновлён";
inline constexpr const char *testProgramUpdated = "Испытание обновлено";
inline constexpr const char *beaufortValueUpdated = "Сила ветра обновлена";
inline constexpr const char *windDirectionUpdated = "Направление ветра обновлено";
inline constexpr const char *angleOfAttackUpdated = "Угол атаки обновлён";
inline constexpr const char *angleOfAttackModelSettingUpdated = "Настройка модели угла атаки обновлена";
inline constexpr const char *controlPlotRebuilt = "График управляющего воздействия перестроен";

inline constexpr const char *estimatedDurationFailed = "Расчёт не выполнен";
inline constexpr const char *estimatedDurationUnknown = "Не рассчитано";
inline constexpr const char *minutesSuffix = " мин";

inline std::string estimatedDurationMinutes(int minutes) {
    return std::to_string(minutes) + minutesSuffix;
}

} // namespace localization::controlCharts

#endif // CONTROLCHARTSSTRINGS_HPP
