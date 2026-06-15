#ifndef ANGLEOFATTACKOSCILLATIONPOLICY_HPP
#define ANGLEOFATTACKOSCILLATIONPOLICY_HPP

#include "WindImpact.hpp"

#include <algorithm>
#include <cmath>

namespace domain {

struct AngleOfAttackSector {
    WindDirection lower{WindDirection::from(0.0)};
    WindDirection upper{WindDirection::from(0.0)};
};

struct AngleOfAttackOscillationState {
    bool initialized{false};
    bool targetIsUpper{true};
    WindDirection center{WindDirection::from(0.0)};
    AngleOfAttack angle{AngleOfAttack::from(0.0)};
    WindDirection currentTarget{WindDirection::from(0.0)};
};

struct AngleOfAttackOscillationStep {
    AngleOfAttackSector sector{};
    WindDirection target{WindDirection::from(0.0)};
    AngleOfAttackOscillationState state{};
};

class AngleOfAttackOscillationPolicy final {
  public:
    [[nodiscard]] static AngleOfAttackSector sector(WindDirection basePosition, AngleOfAttack angle) {
        const double halfAngle = angle.degrees() / 2.0;
        return AngleOfAttackSector{.lower = WindDirection::from(basePosition.degrees() - halfAngle),
                                   .upper = WindDirection::from(basePosition.degrees() + halfAngle)};
    }

    [[nodiscard]] static AngleOfAttackOscillationStep nextTarget(AngleOfAttackOscillationState state,
                                                                 WindDirection basePosition, AngleOfAttack angle) {
        const auto currentPosition = state.initialized ? state.currentTarget : basePosition;
        return nextTarget(state, basePosition, angle, currentPosition);
    }

    [[nodiscard]] static AngleOfAttackOscillationStep nextTarget(AngleOfAttackOscillationState state,
                                                                 WindDirection basePosition, AngleOfAttack angle,
                                                                 WindDirection currentPosition) {
        const auto nextSector = sector(basePosition, angle);

        if (angle.degrees() <= 0.0) {
            return makeStep(nextSector, basePosition,
                            AngleOfAttackOscillationState{.initialized = true,
                                                          .targetIsUpper = true,
                                                          .center = basePosition,
                                                          .angle = angle,
                                                          .currentTarget = basePosition});
        }

        if (!state.initialized || state.center.degrees() != basePosition.degrees() ||
            state.angle.degrees() != angle.degrees()) {
            return makeStep(nextSector, nextSector.upper,
                            AngleOfAttackOscillationState{.initialized = true,
                                                          .targetIsUpper = true,
                                                          .center = basePosition,
                                                          .angle = angle,
                                                          .currentTarget = nextSector.upper});
        }

        if (!hasReached(currentPosition, state.currentTarget)) {
            return makeStep(nextSector, state.currentTarget, state);
        }

        const auto target = state.targetIsUpper ? nextSector.lower : nextSector.upper;
        return makeStep(nextSector, target,
                        AngleOfAttackOscillationState{.initialized = true,
                                                      .targetIsUpper = !state.targetIsUpper,
                                                      .center = basePosition,
                                                      .angle = angle,
                                                      .currentTarget = target});
    }

  private:
    [[nodiscard]] static AngleOfAttackOscillationStep makeStep(AngleOfAttackSector sector, WindDirection target,
                                                               AngleOfAttackOscillationState state) {
        return AngleOfAttackOscillationStep{.sector = sector, .target = target, .state = state};
    }

    [[nodiscard]] static bool hasReached(WindDirection currentPosition, WindDirection target) {
        constexpr double epsilonDegrees = 0.001;
        const double rawDelta = std::abs(currentPosition.degrees() - target.degrees());
        const double circularDelta = std::min(rawDelta, 360.0 - rawDelta);
        return circularDelta <= epsilonDegrees;
    }
};

} // namespace domain

#endif // ANGLEOFATTACKOSCILLATIONPOLICY_HPP
