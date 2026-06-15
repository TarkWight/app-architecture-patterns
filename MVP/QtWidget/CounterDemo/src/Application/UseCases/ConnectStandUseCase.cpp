#include "ConnectStandUseCase.hpp"

#include "../../Domain/StandConnectionTransitions.hpp"

namespace application::useCases {

ConnectStandUseCase::ConnectStandUseCase(application::session::SessionState &state,
                                         application::ports::ITelemetryClient &telemetryClient)
    : state(state), telemetryClient(telemetryClient) {
}

void ConnectStandUseCase::execute() {
    if (!domain::canConnect(state.get().standConnectionStatus)) {
        return;
    }

    state.setStandConnectionStatus(domain::StandConnectionStatus::Connecting);

    telemetryClient.connectAll();
}

} // namespace application::useCases
