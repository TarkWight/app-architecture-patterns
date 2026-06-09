#include "SetWindImpactUseCase.hpp"

namespace application::useCases {

SetWindImpactUseCase::SetWindImpactUseCase(application::session::SessionState &state) : state(state) {
}

void SetWindImpactUseCase::execute(domain::WindImpact profile) {
    state.setWindImpact(std::move(profile));
}

} // namespace application::useCases
