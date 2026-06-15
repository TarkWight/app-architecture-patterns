#include "ApplyBeaufortImpactUseCase.hpp"

#include "../../Domain/StandScenario.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::useCases {

ApplyBeaufortImpactUseCase::ApplyBeaufortImpactUseCase(application::session::SessionState &state) : state(state) {
}

bool ApplyBeaufortImpactUseCase::execute(domain::Beaufort beaufort) {
    const auto &session = state.get();
    if (!domain::StandScenario{session.standControlMode}.allowsManualImpact()) {
        return false;
    }

    state.setTargetStandImpact(session.targetStandImpact.withBeaufort(beaufort));
    return true;
}

} // namespace application::useCases
