#include "StopTimerUseCase.hpp"

namespace application::useCases {

StopTimerUseCase::StopTimerUseCase(application::session::SessionState &state,
                                   application::ports::ITimerService &timerService)
    : state(state), timerService(timerService) {
}

void StopTimerUseCase::execute() {
    timerService.stop();
    state.setTimerRunning(false);
}

} // namespace application::useCases
