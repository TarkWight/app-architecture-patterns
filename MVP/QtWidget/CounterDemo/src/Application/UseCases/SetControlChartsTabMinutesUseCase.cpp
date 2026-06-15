#include "SetControlChartsTabMinutesUseCase.hpp"

#include "../../Domain/TestModeStatePolicy.hpp"

namespace application::useCases {

SetControlChartsTabMinutesUseCase::SetControlChartsTabMinutesUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetControlChartsTabMinutesUseCase::execute(domain::DurationMinutes minutes) {
    const auto &session = state.get();
    if (!domain::TestModeStatePolicy::allowsOperatorDuration(session.testProtocol.testMode)) {
        return;
    }

    state.setControlChartsTabMinutes(minutes);
    state.setOperatorTestDurationMinutes(minutes);
    state.setTestTimeSource(domain::TestModeStatePolicy::timeSourceAfterOperatorDuration(session.testProtocol.testMode,
                                                                                         session.testTimeSource));
}

} // namespace application::useCases
