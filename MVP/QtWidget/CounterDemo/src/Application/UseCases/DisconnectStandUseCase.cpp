#include "DisconnectStandUseCase.hpp"

#include "../../Domain/StandConnectionTransitions.hpp"

namespace application::useCases {

DisconnectStandUseCase::DisconnectStandUseCase(application::session::SessionState &state,
                                               application::ports::ITelemetryClient &telemetryClient)
    : state(state), telemetryClient(telemetryClient) {
}

void DisconnectStandUseCase::execute() {
    if (!domain::canDisconnect(state.get().standConnectionStatus)) {
        return;
    }

    state.setStandConnectionStatus(domain::StandConnectionStatus::Disconnecting);

    telemetryClient.stopPolling();
    telemetryClient.disconnectAll();

    state.setStandConnectionStatus(domain::StandConnectionStatus::Disconnected);
}

} // namespace application::useCases
