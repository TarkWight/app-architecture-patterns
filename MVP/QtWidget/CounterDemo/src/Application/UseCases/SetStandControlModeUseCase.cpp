#include "SetStandControlModeUseCase.hpp"

#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"

namespace application::useCases {

SetStandControlModeUseCase::SetStandControlModeUseCase(application::session::SessionState &state) : state(state) {
}

void SetStandControlModeUseCase::execute(domain::StandControlMode mode) {
    switch (mode) {
    case domain::StandControlMode::Manual:
        state.setTestModeState(domain::testModeForStandControlMode(mode), mode, domain::TestTimeSource::FreeRun,
                               domain::TestTimeDirection::CountUp);
        break;
    case domain::StandControlMode::Hybrid:
    case domain::StandControlMode::PresetScenario:
        state.setTestModeState(domain::testModeForStandControlMode(mode), mode, domain::TestTimeSource::AutoCalculated,
                               domain::TestTimeDirection::CountDown);
        break;
    }
}

} // namespace application::useCases
