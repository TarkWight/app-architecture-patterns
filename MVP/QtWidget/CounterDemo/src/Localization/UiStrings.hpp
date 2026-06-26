#ifndef UISTRINGS_HPP
#define UISTRINGS_HPP

#include <array>
#include <string>

namespace localization::ui {

inline constexpr const char *testTimeSourceAutoCalculated = "Авторасчёт";
inline constexpr const char *testTimeSourceOperatorDefined = "Время оператора";
inline constexpr const char *testTimeSourceFreeRun = "Свободный режим";

inline constexpr const char *tabTelemetry = "Телеметрия";
inline constexpr const char *tabControlCharts = "Управляющие воздействия";
inline constexpr const char *tabProtocol = "Протокол";

inline constexpr std::array<const char *, 16> compassDirectionLabels{
    "С", "ССВ", "СВ", "ВСВ", "В", "ВЮВ", "ЮВ", "ЮЮВ", "Ю", "ЮЮЗ", "ЮЗ", "ЗЮЗ", "З", "ЗСЗ", "СЗ", "ССЗ",
};

inline constexpr const char *telemetryAxisY = "Ось Y / тангаж";
inline constexpr const char *telemetryAxisZ = "Ось Z / направление";

inline constexpr const char *unavailableSelectedTelemetryConfigLog = "Выбранный telemetry.toml недоступен: ";
inline constexpr const char *telemetryConfigUnavailableTitle = "telemetry.toml недоступен";
inline constexpr const char *telemetryConfigUnavailableMessage =
    "Выбранный файл telemetry.toml недоступен. Выберите существующий файл.";
inline constexpr const char *telemetryConfigNotFoundLog = "telemetry.toml не найден: ";
inline constexpr const char *telemetryConfigNotFoundTitle = "telemetry.toml не найден";
inline constexpr const char *telemetryConfigNotFoundMessage =
    "Файл telemetry.toml не найден рядом с приложением. Создайте шаблон и заполните параметры подключения стенда или "
    "выберите файл вручную.";

inline constexpr const char *beaufortParameter = "Бофорт";
inline constexpr const char *directionParameter = "направление";
inline constexpr const char *angleOfAttackParameter = "угол атаки";
inline constexpr const char *manualControlBlockedMessage =
    "Ручное управление стендом недоступно для текущего типа испытания.";
inline constexpr const char *manualControlBlockedTitle = "Ручное управление заблокировано";
inline constexpr const char *hybridManualOverrideAccepted =
    "Гибридный режим принял ручное воздействие как временное отклонение от сценария.";

inline constexpr const char *createTelemetryTemplateDialogTitle = "Создать telemetry.toml";
inline constexpr const char *selectTelemetryConfigDialogTitle = "Выбрать telemetry.toml";
inline constexpr const char *tomlFileFilter = "Файлы TOML (*.toml);;Все файлы (*)";
inline constexpr const char *telemetryTemplateCreatedLog = "Шаблон telemetry.toml создан или уже существует: ";
inline constexpr const char *telemetryTemplateCreationFailedLog = "Не удалось создать шаблон telemetry.toml: ";
inline constexpr const char *templateCreationErrorTitle = "Ошибка создания шаблона";
inline constexpr const char *telemetryConfigSelectedLog = "Выбран telemetry.toml: ";

inline constexpr const char *telemetryTailButton = "К последним данным";

inline constexpr const char *pdfFieldsHeader = "Данные PDF-протокола, организации и результата";
inline constexpr const char *loadTomlButton = "Загрузить TOML";
inline constexpr const char *loadTomlTooltip = "Подгрузить значения полей протокола из .toml файла";
inline constexpr const char *saveTomlTemplateButton = "Создать шаблон TOML";
inline constexpr const char *saveTomlTemplateTooltip = "Сохранить пустой .toml шаблон для полей протокола";
inline constexpr std::array<const char *, 8> protocolLineLabels{
    "Организация", "Номер лицензии", "Адрес", "ФИО оператора", "Комментарий", "Заключение", "Результат", "Резерв",
};
inline constexpr const char *droneParametersGroup = "Конфигурация БПЛА и параметры теста";
inline constexpr const char *loadPdfTomlDialogTitle = "Загрузить данные PDF из TOML";
inline constexpr const char *createPdfTomlTemplateDialogTitle = "Создать шаблон PDF TOML";
inline constexpr const char *pdfReportTemplateFileName = "pdf_report.template.toml";
inline constexpr const char *exportPdfDialogTitle = "Экспорт PDF";
inline constexpr const char *pdfReportFileName = "testProtocol-report.pdf";
inline constexpr const char *pdfFileFilter = "Файлы PDF (*.pdf)";
inline constexpr const char *exportedPdfPrefix = "Экспортировано: ";

inline constexpr const char *customTestProgram = "Пользовательское";
inline constexpr const char *calmTestProgram = "Полет в штиль";
inline constexpr const char *maxParametersTestProgram = "Определение максимальных параметров";
inline constexpr const char *temporalPerspectiveTestProgram = "Исследование временной перспективы";
inline constexpr const char *attenuatedOscillationTestProgram = "Затухающая осцилляция";
inline constexpr const char *manualTestMode = "Ручное";
inline constexpr const char *hybridTestMode = "Гибридное";
inline constexpr const char *automaticTestMode = "Автоматическое";

inline std::string formattedImpact(const std::string &beaufort, const std::string &directionLabel,
                                   const std::string &directionDegrees, const std::string &angleOfAttack) {
    return "Бофорт=" + beaufort + ", направление=" + directionLabel + " (" + directionDegrees +
           "°), угол атаки=" + angleOfAttack + "°";
}

inline std::string manualImpactAcceptedLog(const std::string &parameter, const std::string &previous,
                                           const std::string &target, const std::string &applied) {
    return "Ручное воздействие принято: параметр " + parameter + ", цель " + previous + " -> " + target +
           ", применено " + applied;
}

inline std::string manualImpactTargetReachedLog(const std::string &impact) {
    return "Целевое ручное воздействие достигнуто: " + impact;
}

} // namespace localization::ui

#endif // UISTRINGS_HPP
