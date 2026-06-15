#include "SetControlChartsTabMinutesUseCase.hpp"

#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/Time.hpp"

namespace application::useCases {

SetControlChartsTabMinutesUseCase::SetControlChartsTabMinutesUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetControlChartsTabMinutesUseCase::execute(int minutes) {
    const auto &session = state.get();
    if (session.testProtocol.testMode != domain::TestMode::Hybrid) {
        return;
    }

    const auto duration = domain::DurationMinutes::required(minutes);
    state.setControlChartsTabMinutes(duration);
    state.setOperatorTestDurationMinutes(duration);
    state.setTestTimeSource(domain::TestTimeSource::OperatorDefined);
}

} // namespace application::useCases
