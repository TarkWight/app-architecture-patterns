#ifndef WINDIMPACT_HPP
#define WINDIMPACT_HPP

#include <algorithm>
#include <cmath>

namespace domain {

constexpr double minOperationalBeaufort = 0.0;
// Operational test scope is limited to Beaufort 0..7; drones are not tested above this range.
constexpr double maxOperationalBeaufort = 7.0;
constexpr double directionNormalizationEpsilon = 1.0e-6;
// TODO(post-MVP): verify real stand limits; signed +/-360 is an MVP relative-angle guard.
constexpr double minAngleOfAttack = -360.0;
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
        if (std::abs(rawValue) < directionNormalizationEpsilon) {
            return WindDirection{0.0};
        }

        auto normalized = std::fmod(rawValue, 360.0);
        if (normalized < 0.0) {
            normalized += 360.0;
        }
        if (std::abs(normalized) < directionNormalizationEpsilon ||
            std::abs(normalized - 360.0) < directionNormalizationEpsilon) {
            normalized = 0.0;
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

    [[nodiscard]] WindImpact withBeaufort(Beaufort value) const {
        return WindImpact{.beaufort = value, .direction = direction, .angleOfAttack = angleOfAttack};
    }

    [[nodiscard]] WindImpact withDirection(WindDirection value) const {
        return WindImpact{.beaufort = beaufort, .direction = value, .angleOfAttack = angleOfAttack};
    }

    [[nodiscard]] WindImpact withAngleOfAttack(AngleOfAttack value) const {
        return WindImpact{.beaufort = beaufort, .direction = direction, .angleOfAttack = value};
    }
};

inline WindImpact makeWindImpact(double beaufort, double direction, double angleOfAttack) {
    return WindImpact{.beaufort = Beaufort::from(beaufort),
                      .direction = WindDirection::from(direction),
                      .angleOfAttack = AngleOfAttack::from(angleOfAttack)};
}

} // namespace domain

#endif // WINDIMPACT_HPP
