#include "SetUseAngleOfAttackModelUseCase.hpp"

namespace application::useCases {

SetUseAngleOfAttackModelUseCase::SetUseAngleOfAttackModelUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetUseAngleOfAttackModelUseCase::execute(bool enabled) {
    state.setUseAngleOfAttackModel(enabled);
}

} // namespace application::useCases
