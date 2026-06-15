#include "ApplyWindDirectionUseCase.hpp"

#include "../../Domain/StandScenario.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::useCases {

ApplyWindDirectionUseCase::ApplyWindDirectionUseCase(application::session::SessionState &state) : state(state) {
}

bool ApplyWindDirectionUseCase::execute(double directionDegrees) {
    const auto &session = state.get();
    if (!domain::StandScenario{session.standControlMode}.allowsManualImpact()) {
        return false;
    }

    state.setTargetStandImpact(session.targetStandImpact.withDirection(domain::WindDirection::from(directionDegrees)));
    return true;
}

} // namespace application::useCases
