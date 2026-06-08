#include "SetControlChartsTabMinutesUseCase.hpp"

namespace application::useCases {

SetControlChartsTabMinutesUseCase::SetControlChartsTabMinutesUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetControlChartsTabMinutesUseCase::execute(int minutes) {
    state.setControlChartsTabMinutes(minutes);
    state.setOperatorTestDurationMinutes(minutes);
    state.setTestTimeSource(domain::TestTimeSource::OperatorDefined);
}

} // namespace application::useCases
