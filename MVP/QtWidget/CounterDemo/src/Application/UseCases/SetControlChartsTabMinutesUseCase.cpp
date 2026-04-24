#include "SetControlChartsTabMinutesUseCase.hpp"

namespace application::useCases {

SetControlChartsTabMinutesUseCase::SetControlChartsTabMinutesUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetControlChartsTabMinutesUseCase::execute(int minutes) {
    state.setControlChartsTabMinutes(minutes);
}

} // namespace application::useCases
