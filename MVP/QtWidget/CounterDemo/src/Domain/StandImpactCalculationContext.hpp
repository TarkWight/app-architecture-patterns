#ifndef STANDIMPACTCALCULATIONCONTEXT_HPP
#define STANDIMPACTCALCULATIONCONTEXT_HPP

#include "Time.hpp"
#include "UavSpecification.hpp"
#include "WindImpact.hpp"

#include <optional>

namespace domain {

struct StandImpactCalculationContext {
    WindImpact impact{};
    ElapsedSeconds elapsed{ElapsedSeconds::from(0)};
    std::optional<UavSpecification> uavSpecification{};
};

} // namespace domain

#endif // STANDIMPACTCALCULATIONCONTEXT_HPP
