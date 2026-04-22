#include "SetTestTimeSourceUseCase.hpp"

#include "../../Domain/TestTimeDirection.hpp"

namespace application::useCases {

SetTestTimeSourceUseCase::SetTestTimeSourceUseCase(application::session::SessionState &state)
    : state(state) {
}

void SetTestTimeSourceUseCase::execute(domain::TestTimeSource source) {
    state.setTestTimeSource(source);

    switch (source) {
        case domain::TestTimeSource::AutoCalculated:
        case domain::TestTimeSource::OperatorDefined:
            state.setTestTimeDirection(domain::TestTimeDirection::CountDown);
            break;

        case domain::TestTimeSource::FreeRun:
            state.setTestTimeDirection(domain::TestTimeDirection::CountUp);
            break;
    }
}

} // namespace application::useCases