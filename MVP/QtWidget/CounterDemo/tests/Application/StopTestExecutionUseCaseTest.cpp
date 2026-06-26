#include "../../src/Application/UseCases/StopTestExecutionUseCase.hpp"

#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Ports/ITestExecutionScheduler.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Application/Dto/PlotModel.hpp"
#include "../../src/Domain/StandConnectionStatus.hpp"
#include "../../src/Domain/TestExecutionStatus.hpp"
#include "../../src/Domain/TestTimeDirection.hpp"

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

domain::AxisTelemetrySample validSampleAt(double timestampSeconds) {
    domain::AxisTelemetrySample sample{};
    sample.timestampSeconds = timestampSeconds;
    sample.valid = true;
    return sample;
}

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
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Connected);
    EXPECT_EQ(state.execution().testExecutionStatus, domain::TestExecutionStatus::Ready);
}

TEST(StopTestExecutionUseCaseTest, KeepsTelemetryAndControlPlotsVisible) {
    application::session::SessionState state{};
    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Polling);
    state.appendTelemetrySample(validSampleAt(10.0));
    state.appendControlTraceSample(
        domain::ControlTraceSample{.time = domain::ControlTraceTime::fromSeconds(10.0),
                                   .targetValue = domain::makeWindImpact(4.0, 90.0, 0.0),
                                   .safeCommandValue = domain::makeWindImpact(3.0, 90.0, 0.0)});

    application::dto::PlotModel controlPlot{};
    controlPlot.title = "Управляющее воздействие";
    controlPlot.series.points.push_back(application::dto::Point{.x = 0.0, .y = 1.0});
    state.setControlPlot(controlPlot);

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    application::useCases::StopTestExecutionUseCase useCase{state, scheduler, telemetryClient};

    useCase.execute();

    EXPECT_FALSE(state.telemetry().telemetryHistory.empty());
    EXPECT_EQ(state.control().controlTrace.size(), 1U);
    EXPECT_EQ(state.control().controlPlot.title, "Управляющее воздействие");
    ASSERT_EQ(state.control().controlPlot.series.points.size(), 1U);
    EXPECT_DOUBLE_EQ(state.control().controlPlot.series.points.front().y, 1.0);
}

TEST(StopTestExecutionUseCaseTest, ResetsCountdownTimeUsingDomainStopPlan) {
    application::session::SessionState state{};
    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
    state.setTestTimeDirection(domain::TestTimeDirection::CountDown);
    state.setActiveTestDurationMinutes(domain::DurationMinutes::required(12));
    state.setElapsedSeconds(domain::ElapsedSeconds::from(50));
    state.setRemainingSeconds(domain::RemainingSeconds::from(670));

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    application::useCases::StopTestExecutionUseCase useCase{state, scheduler, telemetryClient};

    useCase.execute();

    EXPECT_EQ(state.execution().elapsed.value(), 0);
    EXPECT_EQ(state.execution().remaining.value(), 720);
    EXPECT_EQ(state.execution().testExecutionStatus, domain::TestExecutionStatus::Ready);
}

TEST(StopTestExecutionUseCaseTest, ResetsCountUpTimeUsingDomainStopPlan) {
    application::session::SessionState state{};
    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
    state.setTestTimeDirection(domain::TestTimeDirection::CountUp);
    state.setActiveTestDurationMinutes(domain::DurationMinutes::required(12));
    state.setElapsedSeconds(domain::ElapsedSeconds::from(50));
    state.setRemainingSeconds(domain::RemainingSeconds::from(0));

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    application::useCases::StopTestExecutionUseCase useCase{state, scheduler, telemetryClient};

    useCase.execute();

    EXPECT_EQ(state.execution().elapsed.value(), 0);
    EXPECT_EQ(state.execution().remaining.value(), 0);
    EXPECT_EQ(state.execution().testExecutionStatus, domain::TestExecutionStatus::Ready);
}

} // namespace
