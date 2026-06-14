#include "DisconnectStandUseCase.hpp"

#include "../../Domain/StandConnectionStatus.hpp"

namespace application::useCases {

DisconnectStandUseCase::DisconnectStandUseCase(application::session::SessionState &state,
                                               application::ports::ITelemetryClient &telemetryClient)
    : state(state), telemetryClient(telemetryClient) {
}

void DisconnectStandUseCase::execute() {
    const auto currentStatus = state.get().standConnectionStatus;
    if (currentStatus == domain::StandConnectionStatus::Disconnected) {
        return;
    }

    state.setStandConnectionStatus(domain::StandConnectionStatus::Disconnecting);

    telemetryClient.stopPolling();
    telemetryClient.disconnectAll();

    state.setStandConnectionStatus(domain::StandConnectionStatus::Disconnected);
}

} // namespace application::useCases
