#include "ConfigureTelemetryUseCase.hpp"

#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/TelemetryConnectionStatus.hpp"

namespace application::useCases {

ConfigureTelemetryUseCase::ConfigureTelemetryUseCase(application::session::SessionState &state,
                                                     application::ports::IConfigRepository &configRepository,
                                                     application::ports::ITelemetryClient &telemetryClient)
    : state(state), configRepository(configRepository), telemetryClient(telemetryClient) {
}

void ConfigureTelemetryUseCase::execute(const std::string &configPath) {
    const auto config = configRepository.loadTelemetryConfig(configPath);

    telemetryClient.setStatusCallback(
        [this](domain::AxisId /*axisId*/, domain::TelemetryConnectionStatus status, const std::string & /*message*/) {
            if (status == domain::TelemetryConnectionStatus::Error) {
                state.setStandConnectionStatus(domain::StandConnectionStatus::Error);
            }
        });

    telemetryClient.setErrorCallback([this](domain::AxisId /*axisId*/, const std::string & /*message*/) {
        state.setStandConnectionStatus(domain::StandConnectionStatus::Error);
    });

    if (config.axis0.enabled) {
        telemetryClient.configureAxis(domain::axis0, config.axis0.host, config.axis0.port);
    }

    if (config.axis1.enabled) {
        telemetryClient.configureAxis(domain::axis1, config.axis1.host, config.axis1.port);
    }

    state.setTelemetryPollIntervalMs(config.pollIntervalMs);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Configured);
}

} // namespace application::useCases
