#include "SetOperatorTestDurationUseCase.hpp"

namespace application::useCases {

SetOperatorTestDurationUseCase::SetOperatorTestDurationUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetOperatorTestDurationUseCase::execute(int minutes) {
    state.setOperatorTestDurationMinutes(minutes);
}

} // namespace application::useCases
