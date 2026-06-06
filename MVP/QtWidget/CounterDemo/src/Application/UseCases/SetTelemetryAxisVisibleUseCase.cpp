#include "SetTelemetryAxisVisibleUseCase.hpp"

namespace application::useCases {

SetTelemetryAxisVisibleUseCase::SetTelemetryAxisVisibleUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetTelemetryAxisVisibleUseCase::execute(domain::AxisId axisId, bool visible) {
    state.setTelemetryAxisVisible(axisId, visible);
}

} // namespace application::useCases
