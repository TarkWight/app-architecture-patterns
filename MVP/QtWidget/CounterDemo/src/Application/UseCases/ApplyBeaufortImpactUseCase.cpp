#include "ApplyBeaufortImpactUseCase.hpp"

#include "../../Domain/HybridBeaufortOverride.hpp"
#include "../../Domain/StandScenario.hpp"
#include "../../Domain/WindControlProfileImpact.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::useCases {

ApplyBeaufortImpactUseCase::ApplyBeaufortImpactUseCase(application::session::SessionState &state) : state(state) {
}

bool ApplyBeaufortImpactUseCase::execute(domain::Beaufort beaufort) {
    const auto &control = state.control();
    const auto scenario = domain::StandScenario{control.standControlMode};
    if (!scenario.allowsManualImpact()) {
        return false;
    }

    if (control.standControlMode == domain::StandControlMode::Hybrid) {
        const auto scenarioImpact =
            domain::windImpactAt(control.controlProfile, state.execution().elapsed, control.targetStandImpact);
        const auto scenarioBeaufort =
            scenarioImpact.has_value() ? scenarioImpact->beaufort : control.targetStandImpact.beaufort;
        state.setHybridBeaufortOverride(
            domain::HybridBeaufortOverridePolicy::startOverride(scenarioBeaufort, beaufort, state.execution().elapsed));
        return true;
    }

    state.setTargetStandImpact(control.targetStandImpact.withBeaufort(beaufort));
    return true;
}

} // namespace application::useCases
