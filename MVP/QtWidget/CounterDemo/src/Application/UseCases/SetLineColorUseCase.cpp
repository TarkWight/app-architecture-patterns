#include "SetLineColorUseCase.hpp"

namespace application::useCases {

SetLineColorUseCase::SetLineColorUseCase(application::session::SessionState &state) : state(state) {
}

void SetLineColorUseCase::execute(application::dto::RgbColor color) {
    state.setLineColor(color);
}

} // namespace application::useCases
