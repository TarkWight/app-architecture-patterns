#include "ConfigureTelemetryUseCase.hpp"

namespace application::useCases {


ConfigureTelemetryUseCase::ConfigureTelemetryUseCase(
    application::ports::IConfigRepository &configRepository,
    application::ports::ITelemetryClient &telemetryClient
    )
    : configRepository(configRepository),
      telemetryClient(telemetryClient) {}

void ConfigureTelemetryUseCase::execute(const std::string &configPath) {
    const auto config = configRepository.loadTelemetryConfig(configPath);

    if (config.axis0.enabled) {
        telemetryClient.configureAxis(
            domain::axis0,
            config.axis0.host,
            config.axis0.port
        );
    }

    if (config.axis1.enabled) {
        telemetryClient.configureAxis(
            domain::axis1,
            config.axis1.host,
            config.axis1.port
        );
    }

    telemetryClient.connectAll();
    telemetryClient.startPolling(config.pollIntervalMs);
}

} // namespace application::useCases