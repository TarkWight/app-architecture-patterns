#include "ApplyAngleOfAttackUseCase.hpp"

#include "../../Domain/StandScenario.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::useCases {

ApplyAngleOfAttackUseCase::ApplyAngleOfAttackUseCase(application::session::SessionState &state) : state(state) {
}

bool ApplyAngleOfAttackUseCase::execute(domain::AngleOfAttack angleOfAttack) {
    const auto &control = state.control();
    const auto scenario = domain::StandScenario{control.standControlMode};
    if (!scenario.allowsManualImpact()) {
        return false;
    }

    if (control.standControlMode == domain::StandControlMode::Hybrid) {
        state.setHybridOperatorAngleOfAttack(angleOfAttack);
        return true;
    }

    state.setTargetStandImpact(control.targetStandImpact.withAngleOfAttack(angleOfAttack));
    return true;
}

} // namespace application::useCases
