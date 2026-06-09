#include "SetTelemetryWindowUseCase.hpp"

namespace application::useCases {

SetTelemetryWindowUseCase::SetTelemetryWindowUseCase(application::session::SessionState &state) : state(state) {
}

void SetTelemetryWindowUseCase::execute(double windowEndSeconds) {
    state.setTelemetryWindowEndSeconds(windowEndSeconds);
}

void SetTelemetryWindowUseCase::followTail() {
    state.followTelemetryTail();
}

} // namespace application::useCases
