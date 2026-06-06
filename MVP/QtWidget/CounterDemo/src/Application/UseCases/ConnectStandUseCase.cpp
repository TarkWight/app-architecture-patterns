#include "ConnectStandUseCase.hpp"

#include "../../Domain/StandConnectionStatus.hpp"

namespace application::useCases {

ConnectStandUseCase::ConnectStandUseCase(application::session::SessionState &state,
                                         application::ports::ITelemetryClient &telemetryClient)
    : state(state), telemetryClient(telemetryClient) {
}

void ConnectStandUseCase::execute() {
    const auto currentStatus = state.get().standConnectionStatus;
    if (currentStatus == domain::StandConnectionStatus::Connecting ||
        currentStatus == domain::StandConnectionStatus::Polling) {
        return;
    }

    state.setStandConnectionStatus(domain::StandConnectionStatus::Connecting);

    telemetryClient.connectAll();
    telemetryClient.startPolling(state.get().telemetryPollIntervalMs);

    state.setStandConnectionStatus(domain::StandConnectionStatus::Polling);
}

} // namespace application::useCases
