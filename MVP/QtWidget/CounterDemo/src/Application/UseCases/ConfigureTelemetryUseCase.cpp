#include "ConfigureTelemetryUseCase.hpp"

#include "../../Domain/StandConnectionStatus.hpp"
#include "../../Domain/TelemetryConnectionStatus.hpp"
#include "../../Domain/TelemetryStatus.hpp"

namespace application::useCases {

ConfigureTelemetryUseCase::ConfigureTelemetryUseCase(application::session::SessionState &state,
                                                     application::ports::IConfigRepository &configRepository,
                                                     application::ports::ITelemetryClient &telemetryClient)
    : state(state), configRepository(configRepository), telemetryClient(telemetryClient) {
}

void ConfigureTelemetryUseCase::execute(const std::string &configPath) {
    const auto config = configRepository.loadTelemetryConfig(configPath);

    telemetryClient.setTelemetryCallback([this](const domain::AxisTelemetrySample &sample) {
        state.appendTelemetrySample(sample);
        state.setTelemetryStatus(domain::TelemetryStatus::Valid);
    });

    telemetryClient.setStatusCallback(
        [this](domain::AxisId /*axisId*/, domain::TelemetryConnectionStatus status, const std::string & /*message*/) {
            const auto currentStatus = state.get().standConnectionStatus;

            switch (status) {
            case domain::TelemetryConnectionStatus::Connecting:
                if (currentStatus != domain::StandConnectionStatus::Disconnecting) {
                    state.setStandConnectionStatus(domain::StandConnectionStatus::Connecting);
                }
                break;
            case domain::TelemetryConnectionStatus::Connected:
            case domain::TelemetryConnectionStatus::Polling:
                if (currentStatus != domain::StandConnectionStatus::Disconnecting) {
                    state.setStandConnectionStatus(domain::StandConnectionStatus::Polling);
                    state.setTelemetryStatus(domain::TelemetryStatus::Valid);
                }
                break;
            case domain::TelemetryConnectionStatus::Disconnected:
                if (currentStatus == domain::StandConnectionStatus::Connecting ||
                    currentStatus == domain::StandConnectionStatus::Polling) {
                    state.setStandConnectionStatus(domain::StandConnectionStatus::Error);
                    state.setTelemetryStatus(domain::TelemetryStatus::Unavailable);
                }
                break;
            case domain::TelemetryConnectionStatus::Error:
                state.setStandConnectionStatus(domain::StandConnectionStatus::Error);
                state.setTelemetryStatus(domain::TelemetryStatus::Unavailable);
                break;
            }
        });

    telemetryClient.setErrorCallback([this](domain::AxisId /*axisId*/, const std::string & /*message*/) {
        state.setStandConnectionStatus(domain::StandConnectionStatus::Error);
        state.setTelemetryStatus(domain::TelemetryStatus::Unavailable);
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
