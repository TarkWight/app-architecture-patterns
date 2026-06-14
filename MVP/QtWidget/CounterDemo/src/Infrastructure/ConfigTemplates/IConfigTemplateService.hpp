#ifndef ICONFIGTEMPLATESERVICE_HPP
#define ICONFIGTEMPLATESERVICE_HPP

#include <filesystem>
#include <vector>

namespace infrastructure::configTemplates {

enum class ConfigTemplateType {
    Telemetry,
    PdfReport,
};

struct ConfigTemplateState {
    ConfigTemplateType type{ConfigTemplateType::Telemetry};
    std::filesystem::path path{};
    bool exists{false};
};

class IConfigTemplateService {
  public:
    virtual ~IConfigTemplateService() = default;

    [[nodiscard]] virtual std::filesystem::path pathFor(ConfigTemplateType type) const = 0;
    [[nodiscard]] virtual bool exists(ConfigTemplateType type) const = 0;
    [[nodiscard]] virtual std::vector<ConfigTemplateState> inspectAll() const = 0;

    virtual void createTemplate(ConfigTemplateType type, const std::filesystem::path &path) const = 0;
};

} // namespace infrastructure::configTemplates

#endif // ICONFIGTEMPLATESERVICE_HPP
