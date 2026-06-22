#ifndef UAVSPECIFICATIONMAPPER_HPP
#define UAVSPECIFICATIONMAPPER_HPP

#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/UavSpecification.hpp"

#include <optional>

namespace application::services {

class UavSpecificationMapper final {
  public:
    [[nodiscard]] std::optional<domain::UavSpecification> map(const domain::TestProtocol &protocol) const;
};

} // namespace application::services

#endif // UAVSPECIFICATIONMAPPER_HPP
