#include "../../src/Application/UseCases/StartTestExecutionUseCase.hpp"

#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Ports/ITestExecutionScheduler.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/AxisId.hpp"
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
    }

    void stopPolling() override {
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
    state.setOperatorTestDurationMinutes(20);

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler, telemetryClient,
                                                             buildControlPlotUseCase};

    useCase.execute();

    EXPECT_TRUE(scheduler.started);
    EXPECT_EQ(scheduler.initialElapsed, 0);
    EXPECT_EQ(state.get().testTimeDirection, domain::TestTimeDirection::CountUp);
    EXPECT_EQ(state.get().activeTestDuration.value(), 0);
    EXPECT_EQ(state.get().remaining.value(), 0);
    EXPECT_FALSE(telemetryClient.axis0Command.has_value());
    EXPECT_FALSE(telemetryClient.axis1Command.has_value());
}

TEST(StartTestExecutionUseCaseTest, AutomaticModeSmoothlyFollowsScenarioImpactFromProfile) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setEstimatedTestDurationMinutes(1);
    state.setWindProfile(domain::makeWindProfile(0.0, 90.0, 5.0, domain::Expression{.value = "x"}));

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler, telemetryClient,
                                                             buildControlPlotUseCase};

    useCase.execute();

    ASSERT_TRUE(telemetryClient.axis1Command.has_value());
    EXPECT_FLOAT_EQ(telemetryClient.axis1Command->torque, 0.1F);
    EXPECT_FLOAT_EQ(telemetryClient.axis1Command->position, 2.5F);
    EXPECT_NEAR(state.get().appliedStandImpact.beaufort.value(), 0.1, 0.000001);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.beaufort.value(), 1.0);

    scheduler.tick(1);

    ASSERT_TRUE(telemetryClient.axis0Command.has_value());
    ASSERT_TRUE(telemetryClient.axis1Command.has_value());
    EXPECT_FLOAT_EQ(telemetryClient.axis0Command->torque, 0.26F);
    EXPECT_FLOAT_EQ(telemetryClient.axis1Command->torque, 0.2F);
    EXPECT_FLOAT_EQ(telemetryClient.axis0Command->position, 2.0F);
    EXPECT_FLOAT_EQ(telemetryClient.axis1Command->position, 5.0F);
    EXPECT_NEAR(state.get().appliedStandImpact.beaufort.value(), 0.2, 0.000001);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.beaufort.value(), 2.0);
}

} // namespace
