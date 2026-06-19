#include "../../src/Application/UseCases/ConfigureTelemetryUseCase.hpp"

#include "../../src/Application/Ports/IConfigRepository.hpp"
#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/StandConnectionStatus.hpp"
#include "../../src/Domain/TelemetryStatus.hpp"
#include "../../src/Domain/TestExecutionStatus.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace {

class ConfigRepositoryStub final : public application::ports::IConfigRepository {
  public:
    application::dto::TelemetryConfig loadTelemetryConfig(const std::string & /*path*/) override {
        return telemetryConfig;
    }

    application::dto::PdfReportConfig loadPdfReportConfig(const std::string & /*path*/) override {
        return {};
    }

    void savePdfReportTemplate(const std::string & /*path*/,
                               const application::dto::PdfReportConfig & /*config*/) override {
    }

    application::dto::TelemetryConfig telemetryConfig{};
};

class TelemetryClientSpy final : public application::ports::ITelemetryClient {
  public:
    void setTelemetryCallback(TelemetryCallback callback) override {
        telemetryCallback = std::move(callback);
    }

    void setStatusCallback(StatusCallback callback) override {
        statusCallback = std::move(callback);
    }

    void setErrorCallback(ErrorCallback callback) override {
        errorCallback = std::move(callback);
    }

    void configureAxis(domain::AxisId /*axisId*/, std::string /*host*/, int /*port*/) override {
        ++configureAxisCalls;
    }

    void connectAxis(domain::AxisId /*axisId*/) override {
    }

    void disconnectAxis(domain::AxisId /*axisId*/) override {
    }

    void connectAll() override {
    }

    void disconnectAll() override {
    }

    void startPolling(int intervalMs) override {
        ++startPollingCalls;
        lastPollingIntervalMs = intervalMs;
    }

    void stopPolling() override {
    }

    void setAxisCommand(domain::AxisId /*axisId*/, domain::AxisControlCommand /*command*/) override {
    }

    void pollOnce(domain::AxisId /*axisId*/) override {
    }

    TelemetryCallback telemetryCallback{};
    StatusCallback statusCallback{};
    ErrorCallback errorCallback{};
    int configureAxisCalls{0};
    int startPollingCalls{0};
    int lastPollingIntervalMs{0};
};

TEST(ConfigureTelemetryUseCaseTest, StartsPollingWhenAxisConnectsDuringActiveTest) {
    application::session::SessionState state{};
    ConfigRepositoryStub configRepository{};
    configRepository.telemetryConfig.pollIntervalMs = 250;
    configRepository.telemetryConfig.axis0.enabled = true;
    configRepository.telemetryConfig.axis0.host = "127.0.0.1";
    configRepository.telemetryConfig.axis0.port = 11540;
    TelemetryClientSpy telemetryClient{};
    application::useCases::ConfigureTelemetryUseCase useCase{state, configRepository, telemetryClient};

    useCase.execute("telemetry.toml");
    state.setStandConnectionStatus(domain::StandConnectionStatus::Connecting);
    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);

    telemetryClient.statusCallback(domain::axis0, domain::TelemetryConnectionStatus::Connected, "Connected");

    EXPECT_EQ(telemetryClient.startPollingCalls, 1);
    EXPECT_EQ(telemetryClient.lastPollingIntervalMs, 250);
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Polling);
    EXPECT_EQ(state.telemetry().telemetryStatus, domain::TelemetryStatus::Valid);
}

TEST(ConfigureTelemetryUseCaseTest, ConnectedStatusDoesNotDowngradeActivePolling) {
    application::session::SessionState state{};
    ConfigRepositoryStub configRepository{};
    configRepository.telemetryConfig.axis0.enabled = true;
    TelemetryClientSpy telemetryClient{};
    application::useCases::ConfigureTelemetryUseCase useCase{state, configRepository, telemetryClient};

    useCase.execute("telemetry.toml");
    state.setStandConnectionStatus(domain::StandConnectionStatus::Polling);

    telemetryClient.statusCallback(domain::axis1, domain::TelemetryConnectionStatus::Connected, "Connected");

    EXPECT_EQ(telemetryClient.startPollingCalls, 0);
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Polling);
    EXPECT_EQ(state.telemetry().telemetryStatus, domain::TelemetryStatus::Valid);
}

TEST(ConfigureTelemetryUseCaseTest, ConnectedStatusMarksStandConnectedWhenNoTestIsActive) {
    application::session::SessionState state{};
    ConfigRepositoryStub configRepository{};
    configRepository.telemetryConfig.axis0.enabled = true;
    TelemetryClientSpy telemetryClient{};
    application::useCases::ConfigureTelemetryUseCase useCase{state, configRepository, telemetryClient};

    useCase.execute("telemetry.toml");
    state.setStandConnectionStatus(domain::StandConnectionStatus::Connecting);

    telemetryClient.statusCallback(domain::axis0, domain::TelemetryConnectionStatus::Connected, "Connected");

    EXPECT_EQ(telemetryClient.startPollingCalls, 0);
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Connected);
    EXPECT_EQ(state.telemetry().telemetryStatus, domain::TelemetryStatus::Valid);
}

TEST(ConfigureTelemetryUseCaseTest, ConnectedStatusDoesNotStartPollingWhileTestIsPaused) {
    application::session::SessionState state{};
    ConfigRepositoryStub configRepository{};
    configRepository.telemetryConfig.pollIntervalMs = 250;
    configRepository.telemetryConfig.axis0.enabled = true;
    TelemetryClientSpy telemetryClient{};
    application::useCases::ConfigureTelemetryUseCase useCase{state, configRepository, telemetryClient};

    useCase.execute("telemetry.toml");
    state.setStandConnectionStatus(domain::StandConnectionStatus::Connecting);
    state.setTestExecutionStatus(domain::TestExecutionStatus::Paused);

    telemetryClient.statusCallback(domain::axis0, domain::TelemetryConnectionStatus::Connected, "Connected");

    EXPECT_EQ(telemetryClient.startPollingCalls, 0);
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Connected);
    EXPECT_EQ(state.telemetry().telemetryStatus, domain::TelemetryStatus::Valid);
}

} // namespace
