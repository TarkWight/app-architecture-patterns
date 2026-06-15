#include "SetOperatorTestDurationUseCase.hpp"

#include "../../Domain/Time.hpp"

namespace application::useCases {

SetOperatorTestDurationUseCase::SetOperatorTestDurationUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetOperatorTestDurationUseCase::execute(int minutes) {
    state.setOperatorTestDurationMinutes(domain::DurationMinutes::required(minutes));
}

} // namespace application::useCases
