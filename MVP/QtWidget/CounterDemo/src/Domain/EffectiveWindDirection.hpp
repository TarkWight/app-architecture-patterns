#ifndef EFFECTIVEWINDDIRECTION_HPP
#define EFFECTIVEWINDDIRECTION_HPP

#include "WindImpact.hpp"

namespace domain {

class YawOscillationOffset final {
  public:
    static YawOscillationOffset from(double rawValue) {
        return YawOscillationOffset{rawValue};
    }

    [[nodiscard]] double degrees() const {
        return rawDegrees;
    }

  private:
    explicit YawOscillationOffset(double degrees) : rawDegrees(degrees) {
    }

    double rawDegrees{0.0};
};

class EffectiveWindDirection final {
  public:
    [[nodiscard]] static WindDirection
    from(WindDirection windDirection, AngleOfAttack angleOfAttack,
         YawOscillationOffset yawOscillationOffset = YawOscillationOffset::from(0.0)) {
        return WindDirection::from(windDirection.degrees() + angleOfAttack.degrees() + yawOscillationOffset.degrees());
    }
};

} // namespace domain

#endif // EFFECTIVEWINDDIRECTION_HPP
