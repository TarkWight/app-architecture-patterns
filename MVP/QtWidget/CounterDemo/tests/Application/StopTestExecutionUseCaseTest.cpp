#include "../../src/Application/UseCases/StopTestExecutionUseCase.hpp"

#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Ports/ITestExecutionScheduler.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/StandConnectionStatus.hpp"
#include "../../src/Domain/TestExecutionStatus.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace {

class TestExecutionSchedulerSpy final : public application::ports::ITestExecutionScheduler {
  public:
    void start(int /*initialElapsedSeconds*/, TickCallback onTick) override {
        tick = std::move(onTick);
    }

    void pause() override {
    }

    void resume() override {
    }

    void stop() override {
        ++stopCalls;
    }

    bool isRunning() const override {
        return false;
    }

    bool isPaused() const override {
        return false;
    }

    TickCallback tick{};
    int stopCalls{0};
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
    }

    void connectAxis(domain::AxisId /*axisId*/) override {
    }

    void disconnectAxis(domain::AxisId /*axisId*/) override {
    }

    void connectAll() override {
    }

    void disconnectAll() override {
    }

    void startPolling(int /*intervalMs*/) override {
    }

    void stopPolling() override {
        ++stopPollingCalls;
    }

    void setAxisCommand(domain::AxisId /*axisId*/, domain::AxisControlCommand /*command*/) override {
    }

    void pollOnce(domain::AxisId /*axisId*/) override {
    }

    TelemetryCallback telemetryCallback{};
    StatusCallback statusCallback{};
    ErrorCallback errorCallback{};
    int stopPollingCalls{0};
};

TEST(StopTestExecutionUseCaseTest, StopsTelemetryPollingAndKeepsStandConnected) {
    application::session::SessionState state{};
    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Polling);

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    application::useCases::StopTestExecutionUseCase useCase{state, scheduler, telemetryClient};

    useCase.execute();

    EXPECT_EQ(scheduler.stopCalls, 1);
    EXPECT_EQ(telemetryClient.stopPollingCalls, 1);
    EXPECT_EQ(state.get().standConnectionStatus, domain::StandConnectionStatus::Connected);
    EXPECT_EQ(state.get().testExecutionStatus, domain::TestExecutionStatus::Ready);
}

} // namespace
