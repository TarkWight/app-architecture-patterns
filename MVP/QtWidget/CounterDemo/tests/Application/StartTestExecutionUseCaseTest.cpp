#include "../../src/Application/UseCases/StartTestExecutionUseCase.hpp"

#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Ports/ITestExecutionScheduler.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Application/Services/UavSpecificationMapper.hpp"
#include "../../src/Domain/AxisId.hpp"
#include "../../src/Domain/StandConnectionStatus.hpp"
#include "../../src/Domain/TestDurationEstimator.hpp"
#include "../../src/Domain/TestExecutionStatus.hpp"
#include "../../src/Domain/TestTimeDirection.hpp"
#include "../../src/Domain/TestTimeSource.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

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
            ++axis0CommandCalls;
        } else if (axisId == domain::axis1) {
            axis1Command = command;
            ++axis1CommandCalls;
        }
    }

    void pollOnce(domain::AxisId /*axisId*/) override {
    }

    TelemetryCallback telemetryCallback{};
    StatusCallback statusCallback{};
    ErrorCallback errorCallback{};
    std::optional<domain::AxisControlCommand> axis0Command{};
    std::optional<domain::AxisControlCommand> axis1Command{};
    int axis0CommandCalls{0};
    int axis1CommandCalls{0};
    int startPollingCalls{0};
    int stopPollingCalls{0};
};

class ScenarioFunctionEngine final : public application::ports::IFunctionEngine {
  public:
    double eval(const std::string & /*expr*/, double x) const override {
        return x < (1.0 / 60.0) ? 1.0 : 2.0;
    }
};

std::vector<domain::TestProtocolParameter> validDroneParameters() {
    return {
        {"uav_model", "Модель БАС", "BAS-1"},
        {"uav_total_weight_kg", "Полная масса", "3,5"},
        {"frontal_area_m2", "Фронтальная площадь", "0,2"},
        {"drag_coefficient", "Коэффициент сопротивления", "1,0"},
        {"equipment_current", "Ток оборудования", "1,0"},
        {"battery_capacity_mah", "Емкость", "18000"},
        {"battery_cell_count", "Число ячеек", "6"},
        {"battery_cell_voltage", "Напряжение ячейки", "3,8"},
        {"battery_discharge_rate_c", "C-rate", "6"},
        {"motor_count", "Количество двигателей", "4"},
        {"motor_max_thrust_kg", "Максимальная тяга", "2,0"},
        {"motor_peak_current_a", "Пиковый ток", "30"},
        {"motor_hover_current_a", "Ток висения", "6"},
    };
}

domain::DurationMinutes expectedDuration(const domain::TestProtocol &protocol, const domain::WindImpact &impact) {
    const auto uav = application::services::UavSpecificationMapper{}.map(protocol);
    const auto result = domain::TestDurationEstimator::estimate(domain::TestDurationEstimationContext{
        .uav = *uav,
        .impact = impact,
    });
    return *result.duration;
}

domain::AxisTelemetrySample validSampleAt(double timestampSeconds) {
    domain::AxisTelemetrySample sample{};
    sample.timestampSeconds = timestampSeconds;
    sample.valid = true;
    return sample;
}

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
    EXPECT_EQ(state.execution().testTimeDirection, domain::TestTimeDirection::CountUp);
    EXPECT_EQ(state.execution().activeTestDuration.value(), 0);
    EXPECT_EQ(state.execution().remaining.value(), 0);
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

    ASSERT_EQ(state.control().controlTrace.size(), 1U);
    EXPECT_DOUBLE_EQ(state.control().controlTrace.front().time.seconds(), 0.0);
}

TEST(StartTestExecutionUseCaseTest, StartClearsTelemetryHistoryForNewDisplaySession) {
    application::session::SessionState state{};
    state.setTestExecutionStatus(domain::TestExecutionStatus::Ready);
    state.appendTelemetrySample(validSampleAt(10.0));
    ASSERT_FALSE(state.telemetry().telemetryHistory.empty());

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler, telemetryClient,
                                                             buildControlPlotUseCase};

    useCase.execute();

    EXPECT_TRUE(state.telemetry().telemetryHistory.empty());
    EXPECT_TRUE(state.telemetry().telemetryPlot.series.points.empty());
    ASSERT_EQ(state.telemetry().telemetryPlot.seriesList.size(), 2U);
    EXPECT_TRUE(state.telemetry().telemetryPlot.seriesList.at(0).series.points.empty());
    EXPECT_TRUE(state.telemetry().telemetryPlot.seriesList.at(1).series.points.empty());
}

TEST(StartTestExecutionUseCaseTest, StartBuildsControlPlotForManualMode) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Manual);

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler, telemetryClient,
                                                             buildControlPlotUseCase};

    useCase.execute();

    EXPECT_EQ(state.control().controlPlot.title, "Control chart");
    EXPECT_GT(state.control().controlPlot.x.max, state.control().controlPlot.x.min);
}

TEST(StartTestExecutionUseCaseTest, InitialSchedulerTickDoesNotDuplicateScenarioTraceOrSend) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler, telemetryClient,
                                                             buildControlPlotUseCase};

    useCase.execute();
    ASSERT_TRUE(scheduler.tick);
    scheduler.tick(0);

    ASSERT_EQ(state.control().controlTrace.size(), 1U);
    EXPECT_DOUBLE_EQ(state.control().controlTrace.front().time.seconds(), 0.0);
    EXPECT_EQ(telemetryClient.axis0CommandCalls, 1);
    EXPECT_EQ(telemetryClient.axis1CommandCalls, 1);
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
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Polling);
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
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Connected);
    EXPECT_EQ(state.execution().testExecutionStatus, domain::TestExecutionStatus::Completed);
}

TEST(StartTestExecutionUseCaseTest, WhenAutoCalculated_UsesEstimatorDuration) {
    application::session::SessionState state{};
    const auto impact = domain::makeWindImpact(4.0, 0.0, 10.0);
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));
    state.setTestProtocolDroneParameters(validDroneParameters());
    state.setWindImpact(impact);

    TestExecutionSchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler, telemetryClient,
                                                             buildControlPlotUseCase};

    useCase.execute();

    const auto expected = expectedDuration(state.protocol().testProtocol, impact);
    EXPECT_EQ(state.protocol().estimatedTestDuration.value(), expected.value());
    EXPECT_EQ(state.execution().activeTestDuration.value(), expected.value());
    EXPECT_NE(state.execution().activeTestDuration.value(), 1);
}

} // namespace
