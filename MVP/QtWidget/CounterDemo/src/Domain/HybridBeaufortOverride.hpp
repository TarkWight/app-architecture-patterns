#ifndef HYBRIDBEAUFORTOVERRIDE_HPP
#define HYBRIDBEAUFORTOVERRIDE_HPP

#include "Time.hpp"
#include "WindImpact.hpp"

#include <algorithm>
#include <optional>

namespace domain {

class DurationSeconds final {
  public:
    static DurationSeconds from(double rawSeconds) {
        return DurationSeconds{std::max(0.0, rawSeconds)};
    }

    [[nodiscard]] double value() const {
        return rawValue;
    }

  private:
    explicit DurationSeconds(double seconds) : rawValue(seconds) {
    }

    double rawValue{0.0};
};

struct HybridBeaufortOverrideCalibration {
    DurationSeconds rampToOverride{DurationSeconds::from(2.0)};
    DurationSeconds hold{DurationSeconds::from(1.0)};
    DurationSeconds returnToScenario{DurationSeconds::from(5.0)};
};

struct HybridBeaufortOverride {
    Beaufort startScenarioBeaufort{Beaufort::from(0.0)};
    Beaufort operatorBeaufort{Beaufort::from(0.0)};
    ElapsedSeconds startedAt{ElapsedSeconds::from(0)};
    DurationSeconds rampToOverride{DurationSeconds::from(0.0)};
    DurationSeconds hold{DurationSeconds::from(0.0)};
    DurationSeconds returnToScenario{DurationSeconds::from(0.0)};
};

class HybridBeaufortOverridePolicy final {
  public:
    [[nodiscard]] static HybridBeaufortOverride
    startOverride(Beaufort currentScenarioBeaufort, Beaufort operatorBeaufort, ElapsedSeconds currentElapsed,
                  const HybridBeaufortOverrideCalibration &calibration = {}) {
        return HybridBeaufortOverride{.startScenarioBeaufort = currentScenarioBeaufort,
                                      .operatorBeaufort = operatorBeaufort,
                                      .startedAt = currentElapsed,
                                      .rampToOverride = calibration.rampToOverride,
                                      .hold = calibration.hold,
                                      .returnToScenario = calibration.returnToScenario};
    }

    [[nodiscard]] static Beaufort resolveBeaufort(const HybridBeaufortOverride &overrideState,
                                                  Beaufort currentScenarioBeaufort, ElapsedSeconds currentElapsed) {
        const double elapsed = elapsedSinceStart(overrideState, currentElapsed);
        const double rampEnd = overrideState.rampToOverride.value();
        if (elapsed < rampEnd) {
            return Beaufort::from(interpolate(overrideState.startScenarioBeaufort.value(),
                                              overrideState.operatorBeaufort.value(),
                                              fraction(elapsed, overrideState.rampToOverride.value())));
        }

        const double holdEnd = rampEnd + overrideState.hold.value();
        if (elapsed < holdEnd) {
            return overrideState.operatorBeaufort;
        }

        const double returnElapsed = elapsed - holdEnd;
        if (returnElapsed < overrideState.returnToScenario.value()) {
            return Beaufort::from(interpolate(overrideState.operatorBeaufort.value(), currentScenarioBeaufort.value(),
                                              fraction(returnElapsed, overrideState.returnToScenario.value())));
        }

        return currentScenarioBeaufort;
    }

    [[nodiscard]] static bool isCompleted(const HybridBeaufortOverride &overrideState, ElapsedSeconds currentElapsed) {
        return elapsedSinceStart(overrideState, currentElapsed) >= totalDuration(overrideState);
    }

  private:
    [[nodiscard]] static double elapsedSinceStart(const HybridBeaufortOverride &overrideState,
                                                  ElapsedSeconds currentElapsed) {
        return std::max(0.0, static_cast<double>(currentElapsed.value() - overrideState.startedAt.value()));
    }

    [[nodiscard]] static double totalDuration(const HybridBeaufortOverride &overrideState) {
        return overrideState.rampToOverride.value() + overrideState.hold.value() +
               overrideState.returnToScenario.value();
    }

    [[nodiscard]] static double fraction(double elapsed, double duration) {
        if (duration <= 0.0) {
            return 1.0;
        }

        return std::clamp(elapsed / duration, 0.0, 1.0);
    }

    [[nodiscard]] static double interpolate(double from, double to, double value) {
        return from + (to - from) * value;
    }
};

class HybridImpactResolver final {
  public:
    [[nodiscard]] static WindImpact resolve(Beaufort scenarioBeaufort, WindDirection operatorDirection,
                                            AngleOfAttack operatorAngleOfAttack,
                                            const std::optional<HybridBeaufortOverride> &overrideState,
                                            ElapsedSeconds elapsed) {
        const auto beaufort =
            overrideState.has_value()
                ? HybridBeaufortOverridePolicy::resolveBeaufort(*overrideState, scenarioBeaufort, elapsed)
                : scenarioBeaufort;

        return WindImpact{.beaufort = beaufort, .direction = operatorDirection, .angleOfAttack = operatorAngleOfAttack};
    }
};

} // namespace domain

#endif // HYBRIDBEAUFORTOVERRIDE_HPP
