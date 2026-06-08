#include "SetStandControlModeUseCase.hpp"

#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"

namespace application::useCases {

SetStandControlModeUseCase::SetStandControlModeUseCase(application::session::SessionState &state) : state(state) {
}

void SetStandControlModeUseCase::execute(domain::StandControlMode mode) {
    state.setStandControlMode(mode);
    state.setTestProtocolMode(domain::testModeForStandControlMode(mode));

    switch (mode) {
    case domain::StandControlMode::Manual:
        state.setTestTimeSource(domain::TestTimeSource::FreeRun);
        state.setTestTimeDirection(domain::TestTimeDirection::CountUp);
        break;
    case domain::StandControlMode::Hybrid:
    case domain::StandControlMode::PresetScenario:
        state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
        state.setTestTimeDirection(domain::TestTimeDirection::CountDown);
        break;
    }
}

} // namespace application::useCases
