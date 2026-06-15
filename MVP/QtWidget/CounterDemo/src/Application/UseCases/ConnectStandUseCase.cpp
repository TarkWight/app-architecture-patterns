#include "ConnectStandUseCase.hpp"

#include "../../Domain/StandConnectionTransitions.hpp"

namespace application::useCases {

ConnectStandUseCase::ConnectStandUseCase(application::session::SessionState &state,
                                         application::ports::ITelemetryClient &telemetryClient)
    : state(state), telemetryClient(telemetryClient) {
}

void ConnectStandUseCase::execute() {
    const auto transition = domain::transitionToConnecting(state.get().standConnectionStatus);
    if (!transition.has_value()) {
        return;
    }

    state.setStandConnectionStatus(*transition);

    telemetryClient.connectAll();
}

} // namespace application::useCases
