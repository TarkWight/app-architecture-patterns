#ifndef IAPPFILELOCATIONPROVIDER_HPP
#define IAPPFILELOCATIONPROVIDER_HPP

#include <filesystem>

namespace infrastructure::configTemplates {

class IAppFileLocationProvider {
  public:
    virtual ~IAppFileLocationProvider() = default;

    [[nodiscard]] virtual std::filesystem::path applicationDir() const = 0;
};

} // namespace infrastructure::configTemplates

#endif // IAPPFILELOCATIONPROVIDER_HPP
