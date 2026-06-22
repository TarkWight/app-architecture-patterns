#ifndef YAWOSCILLATIONPOLICY_HPP
#define YAWOSCILLATIONPOLICY_HPP

#include "EffectiveWindDirection.hpp"
#include "StandImpactCalculationContext.hpp"

#include <cmath>

namespace domain {

class YawOscillationPolicy final {
  public:
    [[nodiscard]] static YawOscillationOffset calculate(const StandImpactCalculationContext &context) {
        const auto amplitude = amplitudeDegrees(context);
        const auto phase = (twoPi * static_cast<double>(context.elapsed.value())) / oscillationPeriodSeconds;
        return YawOscillationOffset::from(amplitude * std::sin(phase));
    }

    [[nodiscard]] static double amplitudeDegrees(const StandImpactCalculationContext &context) {
        // TODO(post-MVP): include UavSpecification mass, payload, DR, motor and battery characteristics.
        return context.impact.beaufort.value() * degreesPerBeaufortPoint;
    }

  private:
    static constexpr double degreesPerBeaufortPoint{1.5};
    static constexpr double oscillationPeriodSeconds{12.0};
    static constexpr double twoPi{6.28318530717958647692};
};

} // namespace domain

#endif // YAWOSCILLATIONPOLICY_HPP
