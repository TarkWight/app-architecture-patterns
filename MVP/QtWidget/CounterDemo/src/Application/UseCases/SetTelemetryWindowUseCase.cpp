#include "SetTelemetryWindowUseCase.hpp"

namespace application::useCases {

SetTelemetryWindowUseCase::SetTelemetryWindowUseCase(application::session::SessionState &state) : state(state) {
}

void SetTelemetryWindowUseCase::execute(domain::TelemetryWindowEnd windowEndSeconds) {
    state.setTelemetryWindowEnd(windowEndSeconds);
}

void SetTelemetryWindowUseCase::followTail() {
    state.followTelemetryTail();
}

} // namespace application::useCases
