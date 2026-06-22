#ifndef QTAPPFILELOCATIONPROVIDER_HPP
#define QTAPPFILELOCATIONPROVIDER_HPP

#include "IAppFileLocationProvider.hpp"

namespace infrastructure::configTemplates {

class QtAppFileLocationProvider final : public IAppFileLocationProvider {
  public:
    [[nodiscard]] std::filesystem::path applicationDir() const override;
};

} // namespace infrastructure::configTemplates

#endif // QTAPPFILELOCATIONPROVIDER_HPP
