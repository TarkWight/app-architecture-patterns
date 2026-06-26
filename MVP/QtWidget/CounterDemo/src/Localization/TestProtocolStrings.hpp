#ifndef TESTPROTOCOLSTRINGS_HPP
#define TESTPROTOCOLSTRINGS_HPP

namespace localization::testProtocol {

inline constexpr const char *pdfReportDefaultsLoaded = "Параметры протокола загружены";
inline constexpr const char *pdfReportDefaultsLoadFailedPrefix = "Не удалось загрузить параметры протокола: ";
inline constexpr const char *testProtocolTitleUpdated = "Заголовок протокола обновлён";
inline constexpr const char *testProtocolLineUpdated = "Строка протокола обновлена";
inline constexpr const char *pdfReportFieldsLoadedFromTomlPrefix = "Поля протокола загружены из TOML: ";
inline constexpr const char *pdfReportTomlLoadFailedPrefix = "Не удалось загрузить TOML протокола: ";
inline constexpr const char *emptyPdfReportTomlTemplateSavedPrefix = "Пустой шаблон TOML протокола сохранён: ";
inline constexpr const char *pdfReportTomlTemplateSaveFailedPrefix = "Не удалось сохранить шаблон TOML протокола: ";
inline constexpr const char *pdfExported = "PDF протокол экспортирован";

} // namespace localization::testProtocol

#endif // TESTPROTOCOLSTRINGS_HPP
