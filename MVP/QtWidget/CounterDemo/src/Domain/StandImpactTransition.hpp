#ifndef STANDIMPACTTRANSITION_HPP
#define STANDIMPACTTRANSITION_HPP

#include "WindProfile.hpp"

#include <cmath>

namespace domain {

struct StandImpactTransitionStep {
    WindProfile impact{};
    bool targetReached{false};
};

class StandImpactTransition final {
  public:
    static constexpr double beaufortStep = 0.1;
    static constexpr double directionStepDegrees = 2.5;
    static constexpr double angleOfAttackStepDegrees = 1.0;
    static constexpr double reachedEpsilon = 0.001;

    [[nodiscard]] StandImpactTransitionStep advance(const WindProfile &current, const WindProfile &target) const {
        auto next = makeWindProfile(
            stepTowards(current.beaufort.value(), target.beaufort.value(), beaufortStep),
            stepTowards(current.direction.degrees(), target.direction.degrees(), directionStepDegrees),
            stepTowards(current.angleOfAttack.degrees(), target.angleOfAttack.degrees(), angleOfAttackStepDegrees),
            target.formula);

        return StandImpactTransitionStep{.impact = next, .targetReached = isReached(next, target)};
    }

  private:
    static double stepTowards(double value, double targetValue, double step) {
        const double delta = targetValue - value;
        if (std::abs(delta) <= step) {
            return targetValue;
        }

        return value + (delta > 0.0 ? step : -step);
    }

    static bool isReached(const WindProfile &value, const WindProfile &target) {
        return std::abs(value.beaufort.value() - target.beaufort.value()) < reachedEpsilon &&
               std::abs(value.direction.degrees() - target.direction.degrees()) < reachedEpsilon &&
               std::abs(value.angleOfAttack.degrees() - target.angleOfAttack.degrees()) < reachedEpsilon;
    }
};

} // namespace domain

#endif // STANDIMPACTTRANSITION_HPP
