#include "ApplyWindDirectionUseCase.hpp"

#include "../../Domain/StandScenario.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::useCases {

ApplyWindDirectionUseCase::ApplyWindDirectionUseCase(application::session::SessionState &state) : state(state) {
}

bool ApplyWindDirectionUseCase::execute(domain::WindDirection direction) {
    const auto &control = state.control();
    const auto scenario = domain::StandScenario{control.standControlMode};
    if (!scenario.allowsManualImpact()) {
        return false;
    }

    if (control.standControlMode == domain::StandControlMode::Hybrid) {
        state.setHybridOperatorDirection(direction);
        return true;
    }

    state.setTargetStandImpact(control.targetStandImpact.withDirection(direction));
    return true;
}

} // namespace application::useCases
