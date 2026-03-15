#include "StartTimerUseCase.hpp"

namespace application::useCases {

StartTimerUseCase::StartTimerUseCase(application::session::SessionState &state,
                                     application::ports::ITimerService &timerService)
    : state(state), timerService(timerService) {
}

void StartTimerUseCase::execute() {
    state.setElapsedSeconds(0);
    state.setTimerRunning(true);

    timerService.start([this](int elapsedSeconds) { state.setElapsedSeconds(elapsedSeconds); });
}

} // namespace application::useCases
