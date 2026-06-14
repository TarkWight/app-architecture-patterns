#ifndef CONFIGTEMPLATESERVICE_HPP
#define CONFIGTEMPLATESERVICE_HPP

#include "IAppFileLocationProvider.hpp"
#include "IConfigTemplateService.hpp"

namespace infrastructure::configTemplates {

class ConfigTemplateService final : public IConfigTemplateService {
  public:
    explicit ConfigTemplateService(const IAppFileLocationProvider &locationProvider);

    [[nodiscard]] std::filesystem::path pathFor(ConfigTemplateType type) const override;
    [[nodiscard]] bool exists(ConfigTemplateType type) const override;
    [[nodiscard]] std::vector<ConfigTemplateState> inspectAll() const override;

    void createTemplate(ConfigTemplateType type, const std::filesystem::path &path) const override;

  private:
    const IAppFileLocationProvider &locationProvider;

    static const char *fileNameFor(ConfigTemplateType type);
    static const char *templateContentFor(ConfigTemplateType type);
};

} // namespace infrastructure::configTemplates

#endif // CONFIGTEMPLATESERVICE_HPP
