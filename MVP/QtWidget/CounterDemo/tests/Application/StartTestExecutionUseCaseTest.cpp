#include "../../src/Application/UseCases/StartTestExecutionUseCase.hpp"

#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Ports/ITestExecutionScheduler.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/AxisId.hpp"
#include "../../src/Domain/StandConnectionStatus.hpp"
#include "../../src/Domain/TestExecutionStatus.hpp"
#include "../../src/Domain/TestTimeDirection.hpp"
#include "../../src/Domain/TestTimeSource.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>

namespace {

class TestExecutionSchedulerSpy final : public application::ports::ITestExecutionScheduler {
  public:
    void start(int initialElapsedSeconds, TickCallback onTick) override {
        started = true;
        initialElapsed = initialElapsedSeconds;
        tick = std::move(onTick);
    }

    void pause() override {
    }

    void resume() override {
    }

    void stop() override {
        stopped = true;
    }

    bool isRunning() const override {
        return started && !stopped;
    }

    bool isPaused() const override {
        return false;
    }

    bool started{false};
    bool stopped{false};
    int initialElapsed{-1};
    TickCallback tick{};
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
        ++startPollingCalls;
    }

    void stopPolling() override {
        ++stopPollingCalls;
    }

    void setAxisCommand(domain::AxisId axisId, domain::AxisControlCommand command) override {
        if (axisId == domain::axis0) {
            axis0Command = command;
        } else if (axisId == domain::axis1) {
            axis1Command = command;
        }
    }

    void pollOnce(domain::AxisId /*axisId*/) override {
    }

    TelemetryCallback telemetryCallback{};
    StatusCallback statusCallback{};
    ErrorCallback errorCallback{};
    std::optional<domain::AxisControlCommand> axis0Command{};
    std::optional<domain::AxisControlCommand> axis1Command{};
    int startPollingCalls{0};
    int stopPollingCalls{0};
};

class ScenarioFunctionEngine final : public application::ports::IFunctionEngine {
  public:
    double eval(const std::string & /*expr*/, double x) const override {
        return x < (1.0 / 60.0) ? 1.0 : 2.0;
    }
};

TEST(StartTestExecutionUseCaseTest, ManualModeStartsAsStopwatchEvenWhenOperatorDurationIsSelected) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Manual);
    state.setTestTimeSource(domain::TestTimeSource::OperatorDefined);
    state.setOperatorTestDurationMinutes(domain::DurationMinutes::required(20));

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler, telemetryClient,
                                                             buildControlPlotUseCase};

    useCase.execute();

    EXPECT_TRUE(scheduler.started);
    EXPECT_EQ(scheduler.initialElapsed, 0);
    EXPECT_EQ(state.get().execution.testTimeDirection, domain::TestTimeDirection::CountUp);
    EXPECT_EQ(state.get().execution.activeTestDuration.value(), 0);
    EXPECT_EQ(state.get().execution.remaining.value(), 0);
    EXPECT_FALSE(telemetryClient.axis0Command.has_value());
    EXPECT_FALSE(telemetryClient.axis1Command.has_value());
}

TEST(StartTestExecutionUseCaseTest, ScenarioStartClearsPreviousControlTrace) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));
    state.appendControlTraceSample(
        domain::ControlTraceSample{.time = domain::ControlTraceTime::fromSeconds(42.0),
                                   .targetValue = domain::makeWindImpact(7.0, 0.0, 0.0),
                                   .safeCommandValue = domain::makeWindImpact(6.0, 0.0, 0.0)});

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler, telemetryClient,
                                                             buildControlPlotUseCase};

    useCase.execute();

    ASSERT_EQ(state.get().control.controlTrace.size(), 1U);
    EXPECT_DOUBLE_EQ(state.get().control.controlTrace.front().time.seconds(), 0.0);
}

TEST(StartTestExecutionUseCaseTest, StartsTelemetryPollingWhenStandIsConnected) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Manual);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Connected);

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler, telemetryClient,
                                                             buildControlPlotUseCase};

    useCase.execute();

    EXPECT_EQ(telemetryClient.startPollingCalls, 1);
    EXPECT_EQ(state.get().connection.standConnectionStatus, domain::StandConnectionStatus::Polling);
}

TEST(StartTestExecutionUseCaseTest, StopsTelemetryPollingWhenTimedScenarioCompletes) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));
    state.setStandConnectionStatus(domain::StandConnectionStatus::Connected);

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler, telemetryClient,
                                                             buildControlPlotUseCase};

    useCase.execute();
    scheduler.tick(60);

    EXPECT_EQ(telemetryClient.startPollingCalls, 1);
    EXPECT_EQ(telemetryClient.stopPollingCalls, 1);
    EXPECT_EQ(state.get().connection.standConnectionStatus, domain::StandConnectionStatus::Connected);
    EXPECT_EQ(state.get().execution.testExecutionStatus, domain::TestExecutionStatus::Completed);
}

} // namespace
