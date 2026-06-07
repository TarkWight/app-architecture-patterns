#ifndef STANDSCENARIO_HPP
#define STANDSCENARIO_HPP

#include "StandControlMode.hpp"

namespace domain {

enum class ManualImpactPolicy {
    KeepManualTarget,
    ReturnToScenarioAfterManualImpact,
    RejectManualImpact,
};

class StandScenario final {
  public:
    explicit StandScenario(StandControlMode mode) : modeValue(mode) {
    }

    [[nodiscard]] StandControlMode mode() const {
        return modeValue;
    }

    [[nodiscard]] bool allowsManualImpact() const {
        return manualImpactPolicy() != ManualImpactPolicy::RejectManualImpact;
    }

    [[nodiscard]] bool locksManualControls() const {
        return !allowsManualImpact();
    }

    [[nodiscard]] ManualImpactPolicy manualImpactPolicy() const {
        switch (modeValue) {
        case StandControlMode::Manual:
            return ManualImpactPolicy::KeepManualTarget;
        case StandControlMode::Hybrid:
            return ManualImpactPolicy::ReturnToScenarioAfterManualImpact;
        case StandControlMode::PresetScenario:
            return ManualImpactPolicy::RejectManualImpact;
        }

        return ManualImpactPolicy::RejectManualImpact;
    }

  private:
    StandControlMode modeValue{StandControlMode::Manual};
};

} // namespace domain

#endif // STANDSCENARIO_HPP
