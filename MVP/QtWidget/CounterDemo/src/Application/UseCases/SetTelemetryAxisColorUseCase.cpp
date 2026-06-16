#include "SetTelemetryAxisColorUseCase.hpp"

namespace application::useCases {

SetTelemetryAxisColorUseCase::SetTelemetryAxisColorUseCase(application::session::SessionState &state) : state(state) {
}

void SetTelemetryAxisColorUseCase::execute(domain::AxisId axisId, application::dto::RgbColor color) {
    state.setTelemetryAxisColor(axisId, color);
}

} // namespace application::useCases
