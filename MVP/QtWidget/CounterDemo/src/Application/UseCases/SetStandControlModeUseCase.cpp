#include "SetStandControlModeUseCase.hpp"

#include "../../Domain/TestModeStatePolicy.hpp"

namespace application::useCases {

SetStandControlModeUseCase::SetStandControlModeUseCase(application::session::SessionState &state) : state(state) {
}

void SetStandControlModeUseCase::execute(domain::StandControlMode mode) {
    const auto modeState = domain::TestModeStatePolicy::fromStandControlMode(mode);
    state.setTestModeState(modeState.testMode, modeState.standControlMode, modeState.timeSource,
                           modeState.timeDirection);
}

} // namespace application::useCases
