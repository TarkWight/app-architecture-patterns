#include "SetWindProfileUseCase.hpp"

namespace application::useCases {

SetWindProfileUseCase::SetWindProfileUseCase(application::session::SessionState &state) : state(state) {
}

void SetWindProfileUseCase::execute(domain::WindProfile profile) {
    state.setWindProfile(std::move(profile));
}

} // namespace application::useCases
