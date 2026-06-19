#include "../../src/Application/UseCases/PauseTestExecutionUseCase.hpp"
#include "../../src/Application/UseCases/ResumeTestExecutionUseCase.hpp"

#include "../../src/Application/Dto/PlotModel.hpp"
#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Ports/ITestExecutionScheduler.hpp"
#include "../../src/Application/Services/TelemetrySessionClock.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/StandConnectionStatus.hpp"
#include "../../src/Domain/TestExecutionStatus.hpp"
#include "../../src/Domain/TelemetryPollInterval.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace {

class TestExecutionSchedulerSpy final : public application::ports::ITestExecutionScheduler {
  public:
    void start(int /*initialElapsedSeconds*/, TickCallback onTick) override {
        tick = std::move(onTick);
        running = true;
        paused = false;
    }

    void pause() override {
        ++pauseCalls;
        running = false;
        paused = true;
    }

    void resume() override {
        ++resumeCalls;
        running = true;
        paused = false;
    }

    void stop() override {
        running = false;
        paused = false;
    }

    bool isRunning() const override {
        return running;
    }

    bool isPaused() const override {
        return paused;
    }

    TickCallback tick{};
    int pauseCalls{0};
    int resumeCalls{0};
    bool running{false};
    bool paused{false};
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

    void startPolling(int intervalMs) override {
        ++startPollingCalls;
        lastPollingIntervalMs = intervalMs;
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
    int startPollingCalls{0};
    int stopPollingCalls{0};
    int lastPollingIntervalMs{0};
};

domain::AxisTelemetrySample validSampleAt(double timestampSeconds) {
    domain::AxisTelemetrySample sample{};
    sample.timestampSeconds = timestampSeconds;
    sample.valid = true;
    return sample;
}

TEST(PauseTestExecutionUseCaseTest, StopsTelemetryPollingAndKeepsPlots) {
    application::session::SessionState state{};
    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Polling);
    state.appendTelemetrySample(validSampleAt(10.0));

    application::dto::PlotModel controlPlot{};
    controlPlot.title = "Control chart";
    controlPlot.series.points.push_back(application::dto::Point{.x = 0.0, .y = 1.0});
    state.setControlPlot(controlPlot);

    TestExecutionSchedulerSpy scheduler{};
    scheduler.running = true;
    TelemetryClientSpy telemetryClient{};
    application::useCases::PauseTestExecutionUseCase useCase{state, scheduler, telemetryClient};

    useCase.execute();

    EXPECT_EQ(scheduler.pauseCalls, 1);
    EXPECT_EQ(telemetryClient.stopPollingCalls, 1);
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Connected);
    EXPECT_EQ(state.execution().testExecutionStatus, domain::TestExecutionStatus::Paused);
    EXPECT_FALSE(state.telemetry().telemetryHistory.empty());
    EXPECT_EQ(state.control().controlPlot.title, "Control chart");
    ASSERT_EQ(state.control().controlPlot.series.points.size(), 1U);
}

TEST(PauseResumeTestExecutionUseCaseTest, FreezesTelemetryLogicalClockBetweenPauseAndResume) {
    application::session::SessionState state{};
    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Polling);
    application::services::TelemetrySessionClock telemetrySessionClock{};
    ASSERT_TRUE(telemetrySessionClock.map(validSampleAt(100.0)).has_value());
    ASSERT_TRUE(telemetrySessionClock.map(validSampleAt(110.0)).has_value());

    TestExecutionSchedulerSpy scheduler{};
    scheduler.running = true;
    TelemetryClientSpy telemetryClient{};
    application::useCases::PauseTestExecutionUseCase pauseUseCase{state, scheduler, telemetryClient,
                                                                  telemetrySessionClock};

    pauseUseCase.execute();
    EXPECT_FALSE(telemetrySessionClock.map(validSampleAt(160.0)).has_value());

    application::useCases::ResumeTestExecutionUseCase resumeUseCase{state, scheduler, telemetryClient,
                                                                    telemetrySessionClock};
    resumeUseCase.execute();
    const auto sample = telemetrySessionClock.map(validSampleAt(161.0));

    ASSERT_TRUE(sample.has_value());
    EXPECT_DOUBLE_EQ(sample->timestampSeconds, 11.0);
}

TEST(ResumeTestExecutionUseCaseTest, RestartsPollingWhenStandIsConnected) {
    application::session::SessionState state{};
    state.setTestExecutionStatus(domain::TestExecutionStatus::Paused);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Connected);
    state.setTelemetryPollInterval(domain::TelemetryPollInterval::fromMilliseconds(250));

    TestExecutionSchedulerSpy scheduler{};
    scheduler.paused = true;
    TelemetryClientSpy telemetryClient{};
    application::useCases::ResumeTestExecutionUseCase useCase{state, scheduler, telemetryClient};

    useCase.execute();

    EXPECT_EQ(scheduler.resumeCalls, 1);
    EXPECT_EQ(telemetryClient.startPollingCalls, 1);
    EXPECT_EQ(telemetryClient.lastPollingIntervalMs, 250);
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Polling);
    EXPECT_EQ(state.execution().testExecutionStatus, domain::TestExecutionStatus::Running);
}

TEST(ResumeTestExecutionUseCaseTest, DoesNotStartPollingWhenStandIsNotConnected) {
    application::session::SessionState state{};
    state.setTestExecutionStatus(domain::TestExecutionStatus::Paused);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Disconnected);

    TestExecutionSchedulerSpy scheduler{};
    scheduler.paused = true;
    TelemetryClientSpy telemetryClient{};
    application::useCases::ResumeTestExecutionUseCase useCase{state, scheduler, telemetryClient};

    useCase.execute();

    EXPECT_EQ(scheduler.resumeCalls, 1);
    EXPECT_EQ(telemetryClient.startPollingCalls, 0);
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Disconnected);
    EXPECT_EQ(state.execution().testExecutionStatus, domain::TestExecutionStatus::Running);
}

} // namespace
