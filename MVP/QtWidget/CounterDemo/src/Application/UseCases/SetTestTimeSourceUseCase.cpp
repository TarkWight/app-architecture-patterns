#include "SetTestTimeSourceUseCase.hpp"

#include "../../Domain/TestModeStatePolicy.hpp"

namespace application::useCases {

SetTestTimeSourceUseCase::SetTestTimeSourceUseCase(application::session::SessionState &state) : state(state) {
}

void SetTestTimeSourceUseCase::execute(domain::TestTimeSource source) {
    state.setTestTimeSource(source);
    state.setTestTimeDirection(domain::TestModeStatePolicy::directionForTimeSource(source));
}

} // namespace application::useCases
