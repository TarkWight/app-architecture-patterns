#include "SetTimerDurationUseCase.hpp"

namespace application::useCases {

SetTimerDurationUseCase::SetTimerDurationUseCase(application::session::SessionState &state) : state(state) {
}

void SetTimerDurationUseCase::execute(int minutes) {
    state.setTimerDurationMinutes(minutes);
}

} // namespace application::useCases
