#include "DisconnectStandUseCase.hpp"

#include "../../Domain/StandConnectionTransitions.hpp"

namespace application::useCases {

DisconnectStandUseCase::DisconnectStandUseCase(application::session::SessionState &state,
                                               application::ports::ITelemetryClient &telemetryClient)
    : state(state), telemetryClient(telemetryClient) {
}

void DisconnectStandUseCase::execute() {
    const auto transition = domain::transitionToDisconnecting(state.get().standConnectionStatus);
    if (!transition.has_value()) {
        return;
    }

    state.setStandConnectionStatus(*transition);

    telemetryClient.stopPolling();
    telemetryClient.disconnectAll();

    const auto completion = domain::transitionAfterDisconnectCompleted(state.get().standConnectionStatus);
    if (completion.has_value()) {
        state.setStandConnectionStatus(*completion);
    }
}

} // namespace application::useCases
