#include "SetControlChartsTabMinutesUseCase.hpp"

#include "../../Domain/TestProtocol.hpp"

namespace application::useCases {

SetControlChartsTabMinutesUseCase::SetControlChartsTabMinutesUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetControlChartsTabMinutesUseCase::execute(int minutes) {
    const auto &session = state.get();
    if (session.testProtocol.testMode != domain::TestMode::Hybrid) {
        return;
    }

    state.setControlChartsTabMinutes(minutes);
    state.setOperatorTestDurationMinutes(minutes);
    state.setTestTimeSource(domain::TestTimeSource::OperatorDefined);
}

} // namespace application::useCases
