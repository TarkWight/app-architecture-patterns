#ifndef CONFIGSTRINGS_HPP
#define CONFIGSTRINGS_HPP

#include <string>

namespace localization::config {

inline constexpr const char *configTemplateCreateFailedPrefix = "Не удалось создать шаблон конфигурации: ";
inline constexpr const char *pdfReportTemplateCreatedByProtocolTab =
    "Шаблон PDF-протокола создаётся через вкладку протокола";
inline constexpr const char *pdfReportTomlOpenForWriteFailed =
    "Не удалось открыть TOML-шаблон PDF-протокола для записи";

inline std::string configTemplateCreateFailed(const std::string &path) {
    return std::string{configTemplateCreateFailedPrefix} + path;
}

} // namespace localization::config

#endif // CONFIGSTRINGS_HPP
