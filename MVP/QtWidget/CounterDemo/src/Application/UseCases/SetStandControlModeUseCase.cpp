#include "SetStandControlModeUseCase.hpp"

namespace application::useCases {

SetStandControlModeUseCase::SetStandControlModeUseCase(application::session::SessionState &state) : state(state) {
}

void SetStandControlModeUseCase::execute(domain::StandControlMode mode) {
    state.setStandControlMode(mode);
    state.setTestProtocolMode(domain::testModeForStandControlMode(mode));
}

} // namespace application::useCases
