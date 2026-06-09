#ifndef ICONFIGREPOSITORY_HPP
#define ICONFIGREPOSITORY_HPP

#include "../Dto/PdfReportConfig.hpp"
#include "../Dto/TelemetryConfig.hpp"

#include <string>

namespace application::ports {

class IConfigRepository {
  public:
    virtual ~IConfigRepository() = default;

    virtual application::dto::TelemetryConfig loadTelemetryConfig(const std::string &path) = 0;
    virtual application::dto::PdfReportConfig loadPdfReportConfig(const std::string &path) = 0;
    virtual void savePdfReportTemplate(const std::string &path, const application::dto::PdfReportConfig &config) = 0;
};

} // namespace application::ports

#endif // ICONFIGREPOSITORY_HPP
