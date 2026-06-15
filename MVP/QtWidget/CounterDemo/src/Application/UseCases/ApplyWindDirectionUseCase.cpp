#include "ApplyWindDirectionUseCase.hpp"

#include "../../Domain/StandScenario.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::useCases {

ApplyWindDirectionUseCase::ApplyWindDirectionUseCase(application::session::SessionState &state) : state(state) {
}

bool ApplyWindDirectionUseCase::execute(domain::WindDirection direction) {
    const auto &session = state.get();
    if (!domain::StandScenario{session.standControlMode}.allowsManualImpact()) {
        return false;
    }

    state.setTargetStandImpact(session.targetStandImpact.withDirection(direction));
    return true;
}

} // namespace application::useCases
