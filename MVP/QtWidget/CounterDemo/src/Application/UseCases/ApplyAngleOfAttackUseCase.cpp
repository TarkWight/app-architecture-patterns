#include "ApplyAngleOfAttackUseCase.hpp"

#include "../../Domain/StandScenario.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::useCases {

ApplyAngleOfAttackUseCase::ApplyAngleOfAttackUseCase(application::session::SessionState &state) : state(state) {
}

bool ApplyAngleOfAttackUseCase::execute(double angleOfAttackDegrees) {
    const auto &session = state.get();
    if (!domain::StandScenario{session.standControlMode}.allowsManualImpact()) {
        return false;
    }

    state.setTargetStandImpact(
        session.targetStandImpact.withAngleOfAttack(domain::AngleOfAttack::from(angleOfAttackDegrees)));
    return true;
}

} // namespace application::useCases
