#include "SetOperatorTestDurationUseCase.hpp"

namespace application::useCases {

SetOperatorTestDurationUseCase::SetOperatorTestDurationUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetOperatorTestDurationUseCase::execute(domain::DurationMinutes minutes) {
    state.setOperatorTestDurationMinutes(minutes);
}

} // namespace application::useCases
