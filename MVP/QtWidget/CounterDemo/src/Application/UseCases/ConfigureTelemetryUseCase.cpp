#include "ConfigureTelemetryUseCase.hpp"

#include "../../Domain/StandConnectionTransitions.hpp"
#include "../../Domain/TelemetryConnectionPolicy.hpp"
#include "../../Domain/TelemetryConnectionStatus.hpp"
#include "../../Domain/TelemetryPollInterval.hpp"
#include "../../Domain/TelemetryStatus.hpp"
#include "../../Domain/TestExecutionTransitions.hpp"

namespace application::useCases {

namespace {

bool testExecutionIsActive(const application::session::ExecutionStateData &execution) {
    return execution.testExecutionStatus == domain::TestExecutionStatus::Running;
}

void applyTelemetryConnectionDecision(application::session::SessionState &state,
                                      application::ports::ITelemetryClient &telemetryClient,
                                      const domain::TelemetryConnectionDecision &decision) {
    if (decision.telemetryStatus.has_value()) {
        state.setTelemetryStatus(*decision.telemetryStatus);
    }

    if (decision.shouldStartPolling) {
        telemetryClient.startPolling(state.connection().telemetryPollInterval.milliseconds());
    }

    if (decision.standConnectionStatus.has_value()) {
        state.setStandConnectionStatus(*decision.standConnectionStatus);
    }
}

} // namespace

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
            const auto decision = domain::TelemetryConnectionPolicy::handleStatus(
                state.connection().standConnectionStatus, status, testExecutionIsActive(state.execution()));
            applyTelemetryConnectionDecision(state, telemetryClient, decision);
        });

    telemetryClient.setErrorCallback([this](domain::AxisId /*axisId*/, const std::string & /*message*/) {
        applyTelemetryConnectionDecision(state, telemetryClient, domain::TelemetryConnectionPolicy::failure());
    });

    if (config.axis0.enabled) {
        telemetryClient.configureAxis(domain::axis0, config.axis0.host, config.axis0.port);
    }

    if (config.axis1.enabled) {
        telemetryClient.configureAxis(domain::axis1, config.axis1.host, config.axis1.port);
    }

    state.setTelemetryPollInterval(domain::TelemetryPollInterval::fromMilliseconds(config.pollIntervalMs));
    state.setStandConnectionStatus(domain::transitionAfterTelemetryConfigured());
}

} // namespace application::useCases
