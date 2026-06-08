#ifndef WINDIMPACT_HPP
#define WINDIMPACT_HPP

#include "Expression.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace domain {

constexpr double minOperationalBeaufort = 0.0;
// TODO(post-MVP): verify real stand/wind-model limits; 0..7 is inherited from the legacy application.
constexpr double maxOperationalBeaufort = 7.0;
constexpr double minAngleOfAttack = 0.0;
// TODO(post-MVP): verify real stand limits; 360 is inherited from the legacy application.
constexpr double maxAngleOfAttack = 360.0;

class Beaufort final {
  public:
    static Beaufort from(double rawValue) {
        return Beaufort{std::clamp(rawValue, minOperationalBeaufort, maxOperationalBeaufort)};
    }

    [[nodiscard]] double value() const {
        return rawValue;
    }

  private:
    explicit Beaufort(double value) : rawValue(value) {
    }

    double rawValue{minOperationalBeaufort};
};

class WindDirection final {
  public:
    static WindDirection from(double rawValue) {
        auto normalized = std::fmod(rawValue, 360.0);
        if (normalized < 0.0) {
            normalized += 360.0;
        }

        return WindDirection{normalized};
    }

    [[nodiscard]] double degrees() const {
        return rawDegrees;
    }

  private:
    explicit WindDirection(double degrees) : rawDegrees(degrees) {
    }

    double rawDegrees{0.0};
};

class AngleOfAttack final {
  public:
    static AngleOfAttack from(double rawValue) {
        return AngleOfAttack{std::clamp(rawValue, minAngleOfAttack, maxAngleOfAttack)};
    }

    [[nodiscard]] double degrees() const {
        return rawDegrees;
    }

  private:
    explicit AngleOfAttack(double degrees) : rawDegrees(degrees) {
    }

    double rawDegrees{0.0};
};

struct WindImpact {
    Beaufort beaufort{Beaufort::from(0.0)};
    WindDirection direction{WindDirection::from(0.0)};
    AngleOfAttack angleOfAttack{AngleOfAttack::from(0.0)};
    Expression formula{};
};

inline WindImpact makeWindImpact(double beaufort, double direction, double angleOfAttack, Expression formula = {}) {
    return WindImpact{.beaufort = Beaufort::from(beaufort),
                      .direction = WindDirection::from(direction),
                      .angleOfAttack = AngleOfAttack::from(angleOfAttack),
                      .formula = std::move(formula)};
}

} // namespace domain

#endif // WINDIMPACT_HPP
