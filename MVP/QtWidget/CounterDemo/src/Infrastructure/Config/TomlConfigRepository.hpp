#ifndef TOMLCONFIGREPOSITORY_HPP
#define TOMLCONFIGREPOSITORY_HPP

#include "../../Application/Ports/IConfigRepository.hpp"

namespace infrastructure::config {

class TomlConfigRepository final : public application::ports::IConfigRepository {
  public:
    application::dto::TelemetryConfig loadTelemetryConfig(const std::string &path) override;
    application::dto::PdfReportConfig loadPdfReportConfig(const std::string &path) override;
};

} // namespace infrastructure::config

#endif // TOMLCONFIGREPOSITORY_HPP
