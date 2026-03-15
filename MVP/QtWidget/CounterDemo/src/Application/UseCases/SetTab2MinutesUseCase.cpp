#include "SetTab2MinutesUseCase.hpp"

namespace application::useCases {

SetTab2MinutesUseCase::SetTab2MinutesUseCase(application::session::SessionState &state) : state(state) {
}

void SetTab2MinutesUseCase::execute(int minutes) {
    state.setTab2Minutes(minutes);
}

} // namespace application::useCases
