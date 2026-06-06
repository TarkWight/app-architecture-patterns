#include "SetStandImpactUseCase.hpp"

namespace application::useCases {

SetStandImpactUseCase::SetStandImpactUseCase(application::session::SessionState &state) : state(state) {
}

void SetStandImpactUseCase::setTarget(domain::WindProfile profile) {
    state.setTargetStandImpact(std::move(profile));
}

void SetStandImpactUseCase::setApplied(domain::WindProfile profile) {
    state.setAppliedStandImpact(std::move(profile));
}

} // namespace application::useCases
