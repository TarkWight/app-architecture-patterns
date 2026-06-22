#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Services/UavSpecificationMapper.hpp"
#include "../../src/Application/UseCases/BuildControlPlotUseCase.hpp"
#include "../../src/Application/UseCases/PauseTestExecutionUseCase.hpp"
#include "../../src/Application/UseCases/ResumeTestExecutionUseCase.hpp"
#include "../../src/Application/UseCases/StartTestExecutionUseCase.hpp"
#include "../../src/Domain/TestDurationEstimator.hpp"
#include "../../src/Domain/TestExecutionStatus.hpp"
#include "../../src/Domain/TestTimeSource.hpp"
#include "../../src/Infrastructure/QtTestExecutionScheduler.hpp"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

void ensureQtApplication() {
    if (QCoreApplication::instance() != nullptr) {
        return;
    }

    static int argc = 1;
    static char appName[] = "qt_scheduler_tests";
    static char *argv[] = {appName, nullptr};
    static QCoreApplication app(argc, argv);
}

void waitMs(int milliseconds) {
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

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

TEST(QtTestExecutionSchedulerTest, Scheduler_WhenPaused_DoesNotAdvanceElapsed) {
    ensureQtApplication();
    infrastructure::QtTestExecutionScheduler scheduler{10};
    std::vector<int> ticks{};

    scheduler.start(0, [&ticks](int elapsed) { ticks.push_back(elapsed); });
    waitMs(25);
    scheduler.pause();
    const auto ticksAtPause = ticks;

    waitMs(50);

    EXPECT_EQ(ticks, ticksAtPause);
    scheduler.stop();
}

TEST(QtTestExecutionSchedulerTest, Scheduler_WhenResumed_ContinuesFromPausedElapsed) {
    ensureQtApplication();
    infrastructure::QtTestExecutionScheduler scheduler{10};
    std::vector<int> ticks{};

    scheduler.start(0, [&ticks](int elapsed) { ticks.push_back(elapsed); });
    waitMs(25);
    scheduler.pause();
    ASSERT_FALSE(ticks.empty());
    const int pausedElapsed = ticks.back();

    waitMs(50);
    scheduler.resume();
    waitMs(15);

    ASSERT_FALSE(ticks.empty());
    EXPECT_EQ(ticks.back(), pausedElapsed + 1);
    scheduler.stop();
}

TEST(QtTestExecutionSchedulerTest, Scheduler_WhenPaused_DoesNotEmitTicks) {
    ensureQtApplication();
    infrastructure::QtTestExecutionScheduler scheduler{10};
    int tickCount = 0;

    scheduler.start(0, [&tickCount](int /*elapsed*/) { ++tickCount; });
    scheduler.pause();
    waitMs(25);

    EXPECT_EQ(tickCount, 0);
    scheduler.stop();
}

TEST(QtTestExecutionSchedulerTest, Scheduler_WhenResumed_DoesNotEmitCatchUpTicks) {
    ensureQtApplication();
    infrastructure::QtTestExecutionScheduler scheduler{10};
    std::vector<int> ticks{};

    scheduler.start(0, [&ticks](int elapsed) { ticks.push_back(elapsed); });
    waitMs(15);
    scheduler.pause();
    const auto tickCountAtPause = ticks.size();

    waitMs(60);
    scheduler.resume();
    waitMs(15);

    EXPECT_LE(ticks.size(), tickCountAtPause + 1);
    scheduler.stop();
}

TEST(QtTestExecutionSchedulerTest, StartPauseResume_WhenPausedForSeveralSeconds_NoElapsedJump) {
    ensureQtApplication();
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Manual);

    infrastructure::QtTestExecutionScheduler scheduler{10};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase startUseCase{state, scheduler, telemetryClient,
                                                                  buildControlPlotUseCase};
    application::useCases::PauseTestExecutionUseCase pauseUseCase{state, scheduler, telemetryClient};
    application::useCases::ResumeTestExecutionUseCase resumeUseCase{state, scheduler, telemetryClient};

    startUseCase.execute();
    waitMs(25);
    pauseUseCase.execute();
    const int pausedElapsed = state.execution().elapsed.value();

    waitMs(60);
    EXPECT_EQ(state.execution().elapsed.value(), pausedElapsed);

    resumeUseCase.execute();
    waitMs(15);

    EXPECT_EQ(state.execution().elapsed.value(), pausedElapsed + 1);
    scheduler.stop();
}

TEST(QtTestExecutionSchedulerTest, Scenario_WhenPaused_DoesNotSkipStepsAfterResume) {
    ensureQtApplication();
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));
    state.setTestProtocolDroneParameters(validDroneParameters());

    infrastructure::QtTestExecutionScheduler scheduler{10};
    TelemetryClientSpy telemetryClient{};
    ScenarioFunctionEngine functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::StartTestExecutionUseCase startUseCase{state, scheduler, telemetryClient,
                                                                  buildControlPlotUseCase};
    application::useCases::PauseTestExecutionUseCase pauseUseCase{state, scheduler, telemetryClient};
    application::useCases::ResumeTestExecutionUseCase resumeUseCase{state, scheduler, telemetryClient};

    startUseCase.execute();
    waitMs(25);
    pauseUseCase.execute();
    const auto traceSizeAtPause = state.control().controlTrace.size();
    const int elapsedAtPause = state.execution().elapsed.value();

    waitMs(60);
    EXPECT_EQ(state.control().controlTrace.size(), traceSizeAtPause);
    EXPECT_EQ(state.execution().elapsed.value(), elapsedAtPause);

    resumeUseCase.execute();
    waitMs(15);

    EXPECT_LE(state.control().controlTrace.size(), traceSizeAtPause + 1);
    EXPECT_EQ(state.execution().elapsed.value(), elapsedAtPause + 1);
    scheduler.stop();
}

} // namespace
