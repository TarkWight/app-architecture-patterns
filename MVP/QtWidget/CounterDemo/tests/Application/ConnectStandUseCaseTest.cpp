#include "../../src/Application/UseCases/ConnectStandUseCase.hpp"

#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/StandConnectionStatus.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace {

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
    }

    void connectAxis(domain::AxisId /*axisId*/) override {
    }

    void disconnectAxis(domain::AxisId /*axisId*/) override {
    }

    void connectAll() override {
        ++connectAllCalls;
    }

    void disconnectAll() override {
    }

    void startPolling(int /*intervalMs*/) override {
        ++startPollingCalls;
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
    int connectAllCalls{0};
    int startPollingCalls{0};
};

TEST(ConnectStandUseCaseTest, ConnectsStandWithoutStartingTelemetryPolling) {
    application::session::SessionState state{};
    TelemetryClientSpy telemetryClient{};
    application::useCases::ConnectStandUseCase useCase{state, telemetryClient};

    useCase.execute();

    EXPECT_EQ(telemetryClient.connectAllCalls, 1);
    EXPECT_EQ(telemetryClient.startPollingCalls, 0);
    EXPECT_EQ(state.get().connection.standConnectionStatus, domain::StandConnectionStatus::Connecting);
}

} // namespace
