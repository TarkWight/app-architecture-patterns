#include "../../src/Presentation/ShellPresenter.hpp"

#include "../../src/Application/Ports/IConfigRepository.hpp"
#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Ports/ITestExecutionScheduler.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Application/Services/TelemetrySessionClock.hpp"
#include "../../src/Application/UseCases/BuildControlPlotUseCase.hpp"
#include "../../src/Application/UseCases/CalculateAndBuildControlPlotUseCase.hpp"
#include "../../src/Application/UseCases/ConfigureTelemetryUseCase.hpp"
#include "../../src/Application/UseCases/ConnectStandUseCase.hpp"
#include "../../src/Application/UseCases/DisconnectStandUseCase.hpp"
#include "../../src/Application/UseCases/EstimateTestDurationUseCase.hpp"
#include "../../src/Application/UseCases/PauseTestExecutionUseCase.hpp"
#include "../../src/Application/UseCases/ResumeTestExecutionUseCase.hpp"
#include "../../src/Application/UseCases/SetFunctionExpressionUseCase.hpp"
#include "../../src/Application/UseCases/SetLineColorUseCase.hpp"
#include "../../src/Application/UseCases/SetStandControlModeUseCase.hpp"
#include "../../src/Application/UseCases/SetTestTimeSourceUseCase.hpp"
#include "../../src/Application/UseCases/StartTestExecutionUseCase.hpp"
#include "../../src/Application/UseCases/StopTestExecutionUseCase.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

class FunctionEngineStub final : public application::ports::IFunctionEngine {
  public:
    double eval(const std::string & /*expr*/, double x) const override {
        return x;
    }
};

class SchedulerSpy final : public application::ports::ITestExecutionScheduler {
  public:
    void start(int initialElapsedSeconds, TickCallback onTick) override {
        ++startCount;
        running = true;
        paused = false;
        lastInitialElapsedSeconds = initialElapsedSeconds;
        callback = std::move(onTick);
    }

    void pause() override {
        paused = true;
        running = false;
    }

    void resume() override {
        paused = false;
        running = true;
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

    int startCount{0};
    int lastInitialElapsedSeconds{-1};
    bool running{false};
    bool paused{false};
    TickCallback callback{};
};

class TelemetryClientSpy final : public application::ports::ITelemetryClient {
  public:
    void setTelemetryCallback(TelemetryCallback callbackValue) override {
        telemetryCallback = std::move(callbackValue);
    }

    void setStatusCallback(StatusCallback callbackValue) override {
        statusCallback = std::move(callbackValue);
    }

    void setErrorCallback(ErrorCallback callbackValue) override {
        errorCallback = std::move(callbackValue);
    }

    void setTraceCallback(TraceCallback callbackValue) override {
        traceCallback = std::move(callbackValue);
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

    void setAxisCommand(domain::AxisId /*axisId*/, domain::AxisControlCommand /*command*/) override {
    }

    void pollOnce(domain::AxisId /*axisId*/) override {
    }

    TelemetryCallback telemetryCallback{};
    StatusCallback statusCallback{};
    ErrorCallback errorCallback{};
    TraceCallback traceCallback{};
};

class ConfigRepositoryStub final : public application::ports::IConfigRepository {
  public:
    application::dto::TelemetryConfig loadTelemetryConfig(const std::string & /*path*/) override {
        return {};
    }

    application::dto::PdfReportConfig loadPdfReportConfig(const std::string & /*path*/) override {
        return {};
    }

    void savePdfReportTemplate(const std::string & /*path*/,
                               const application::dto::PdfReportConfig & /*config*/) override {
    }
};

class ShellViewSpy final : public presentation::IShellView {
  public:
    void setTimerText(const std::string &text) override {
        timerText = text;
    }

    void setStartEnabled(bool enabled) override {
        startEnabled = enabled;
    }

    void setStopEnabled(bool enabled) override {
        stopEnabled = enabled;
    }

    void setPauseResumeEnabled(bool enabled) override {
        pauseResumeEnabled = enabled;
    }

    void setPauseResumeText(const std::string &text) override {
        pauseResumeText = text;
    }

    void setStandConnectionButtonText(const std::string &text) override {
        standConnectionButtonText = text;
    }

    void setStandConnectionStatusText(const std::string &text) override {
        standConnectionStatusText = text;
    }

    void setFunctionExpression(const std::string &expression) override {
        functionExpression = expression;
    }

    void setTestTimeSource(domain::TestTimeSource source) override {
        testTimeSource = source;
    }

    void setTestTimeSourceEnabled(bool enabled) override {
        testTimeSourceEnabled = enabled;
    }

    void appendLog(const std::string &text) override {
        logs.push_back(text);
    }

    void showOperatorWarning(const std::string &title, const std::string &message) override {
        lastWarningTitle = title;
        lastWarningMessage = message;
    }

    bool confirmDangerousReadinessStart(const std::string &title, const std::string &message) override {
        ++confirmationCount;
        lastConfirmationTitle = title;
        lastConfirmationMessage = message;
        return confirmationResponse;
    }

    void freezeStandImpactTransition() override {
        ++freezeCount;
    }

    std::string timerText{};
    bool startEnabled{false};
    bool stopEnabled{false};
    bool pauseResumeEnabled{false};
    std::string pauseResumeText{};
    std::string standConnectionButtonText{};
    std::string standConnectionStatusText{};
    std::string functionExpression{};
    domain::TestTimeSource testTimeSource{domain::TestTimeSource::FreeRun};
    bool testTimeSourceEnabled{false};
    std::vector<std::string> logs{};
    std::string lastWarningTitle{};
    std::string lastWarningMessage{};
    bool confirmationResponse{true};
    int confirmationCount{0};
    std::string lastConfirmationTitle{};
    std::string lastConfirmationMessage{};
    int freezeCount{0};
};

std::vector<domain::TestProtocolParameter> validDroneParameters() {
    return {
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

std::vector<domain::TestProtocolParameter> warningDroneParameters() {
    auto parameters = validDroneParameters();
    for (auto &parameter : parameters) {
        if (parameter.key == "frontal_area_m2") {
            parameter.value = "";
        }
    }
    return parameters;
}

std::vector<domain::TestProtocolParameter> invalidDroneParameters() {
    return {
        {"uav_model", "Модель БАС", "BAS-1"},
        {"battery_capacity_mah", "Емкость", "0"},
    };
}

struct ShellPresenterFixture {
    application::session::SessionState state{};
    SchedulerSpy scheduler{};
    TelemetryClientSpy telemetryClient{};
    FunctionEngineStub functionEngine{};
    ConfigRepositoryStub configRepository{};
    application::services::TelemetrySessionClock telemetrySessionClock{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::CalculateAndBuildControlPlotUseCase calculateAndBuildControlPlotUseCase{
        state, functionEngine, buildControlPlotUseCase};
    application::useCases::StartTestExecutionUseCase startUseCase{state, scheduler, telemetryClient,
                                                                  buildControlPlotUseCase};
    application::useCases::PauseTestExecutionUseCase pauseUseCase{state, scheduler, telemetryClient};
    application::useCases::ResumeTestExecutionUseCase resumeUseCase{state, scheduler, telemetryClient};
    application::useCases::StopTestExecutionUseCase stopUseCase{state, scheduler, telemetryClient};
    application::useCases::SetTestTimeSourceUseCase setTestTimeSourceUseCase{state};
    application::useCases::SetFunctionExpressionUseCase setFunctionExpressionUseCase{state};
    application::useCases::SetLineColorUseCase setLineColorUseCase{state};
    application::useCases::SetStandControlModeUseCase setStandControlModeUseCase{state};
    application::useCases::EstimateTestDurationUseCase estimateTestDurationUseCase{state};
    application::useCases::ConfigureTelemetryUseCase configureTelemetryUseCase{state, configRepository, telemetryClient,
                                                                               telemetrySessionClock};
    application::useCases::ConnectStandUseCase connectStandUseCase{state, telemetryClient};
    application::useCases::DisconnectStandUseCase disconnectStandUseCase{state, telemetryClient};
    presentation::ShellPresenter presenter{presentation::ShellPresenter::Dependencies{
        .state = state,
        .startTestExecutionUseCase = startUseCase,
        .pauseTestExecutionUseCase = pauseUseCase,
        .resumeTestExecutionUseCase = resumeUseCase,
        .stopTestExecutionUseCase = stopUseCase,
        .setTestTimeSourceUseCase = setTestTimeSourceUseCase,
        .setFunctionExpressionUseCase = setFunctionExpressionUseCase,
        .setLineColorUseCase = setLineColorUseCase,
        .setStandControlModeUseCase = setStandControlModeUseCase,
        .calculateAndBuildControlPlotUseCase = calculateAndBuildControlPlotUseCase,
        .estimateTestDurationUseCase = estimateTestDurationUseCase,
        .configureTelemetryUseCase = configureTelemetryUseCase,
        .connectStandUseCase = connectStandUseCase,
        .disconnectStandUseCase = disconnectStandUseCase,
        .telemetryClient = telemetryClient}};
    ShellViewSpy view{};
};

void prepareAuto(ShellPresenterFixture &fixture, std::vector<domain::TestProtocolParameter> parameters) {
    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    fixture.state.setTestProtocolDroneParameters(std::move(parameters));
}

void connectStand(ShellPresenterFixture &fixture) {
    fixture.state.setStandConnectionStatus(domain::StandConnectionStatus::Connected);
}

void setReadiness(ShellPresenterFixture &fixture, application::session::ReadinessStatus status) {
    domain::EstimatedTestDurationResult result{};
    if (status != application::session::ReadinessStatus::Failed) {
        result.duration = domain::DurationMinutes::required(12);
    }
    if (status == application::session::ReadinessStatus::Warning) {
        result.warnings.push_back(
            domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::FrontalAreaFallbackUsed});
    }
    if (status == application::session::ReadinessStatus::Dangerous) {
        result.errors.push_back(
            domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::RequiredThrustExceedsAvailable});
    }
    fixture.state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
}

} // namespace

TEST(ShellPresenterTest, ManualStartDoesNotAskReadinessConfirmation) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    connectStand(fixture);
    fixture.state.setTestProtocolMode(domain::TestMode::Manual);
    setReadiness(fixture, application::session::ReadinessStatus::Failed);

    fixture.presenter.onStartPressed();

    EXPECT_EQ(fixture.view.confirmationCount, 0);
    EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Running);
}

TEST(ShellPresenterTest, CalculatePressedUpdatesReadinessAndBuildsControlPlot) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    fixture.state.setTestProtocolDroneParameters(validDroneParameters());
    fixture.state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));

    fixture.presenter.onCalculatePressed();

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Ok);
    EXPECT_FALSE(fixture.state.control().controlProfile.samples.empty());
    EXPECT_FALSE(fixture.state.control().controlPlot.series.points.empty());
    EXPECT_NE(fixture.state.control().controlProfile.duration.value(), 1);
}

TEST(ShellPresenterTest, StandTraceMessageAppendsLog) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);

    ASSERT_TRUE(static_cast<bool>(fixture.telemetryClient.traceCallback));

    fixture.telemetryClient.traceCallback("[STAND][CONNECTED][axis0]");

    ASSERT_FALSE(fixture.view.logs.empty());
    EXPECT_EQ(fixture.view.logs.back(), "[STAND][CONNECTED][axis0]");
}

TEST(ShellPresenterTest, AutoWithOkStartsWithoutConfirmation) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    connectStand(fixture);
    prepareAuto(fixture, validDroneParameters());
    setReadiness(fixture, application::session::ReadinessStatus::Ok);

    fixture.presenter.onStartPressed();

    EXPECT_EQ(fixture.view.confirmationCount, 0);
    EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Running);
}

TEST(ShellPresenterTest, HybridWithWarningStartsWithoutConfirmation) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    connectStand(fixture);
    fixture.state.setTestProtocolMode(domain::TestMode::Hybrid);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    fixture.state.setTestProtocolDroneParameters(warningDroneParameters());
    setReadiness(fixture, application::session::ReadinessStatus::Warning);

    fixture.presenter.onStartPressed();

    EXPECT_EQ(fixture.view.confirmationCount, 0);
    EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Running);
}

TEST(ShellPresenterTest, DangerousReadinessAsksConfirmationAndStartsWhenConfirmed) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    connectStand(fixture);
    prepareAuto(fixture, validDroneParameters());
    setReadiness(fixture, application::session::ReadinessStatus::Dangerous);
    fixture.view.confirmationResponse = true;

    fixture.presenter.onStartPressed();

    EXPECT_EQ(fixture.view.confirmationCount, 1);
    EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Running);
}

TEST(ShellPresenterTest, FailedReadinessAsksConfirmationAndCancelsStart) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    connectStand(fixture);
    prepareAuto(fixture, invalidDroneParameters());
    setReadiness(fixture, application::session::ReadinessStatus::Failed);
    fixture.view.confirmationResponse = false;

    fixture.presenter.onStartPressed();

    EXPECT_EQ(fixture.view.confirmationCount, 1);
    EXPECT_EQ(fixture.scheduler.startCount, 0);
    EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Idle);
}

TEST(ShellPresenterTest, UnknownReadinessRunsEstimatorBeforeStart) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    connectStand(fixture);
    prepareAuto(fixture, validDroneParameters());
    ASSERT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Unknown);

    fixture.presenter.onStartPressed();

    EXPECT_EQ(fixture.view.confirmationCount, 0);
    EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Running);
}

TEST(ShellPresenterTest, UnknownFailedReadinessRunsEstimatorAndAsksConfirmation) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    connectStand(fixture);
    prepareAuto(fixture, invalidDroneParameters());
    fixture.view.confirmationResponse = false;
    ASSERT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Unknown);

    fixture.presenter.onStartPressed();

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Failed);
    EXPECT_EQ(fixture.view.confirmationCount, 1);
    EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Idle);
}

TEST(ShellPresenterTest, DangerousConfirmationIsRequestedAgainOnNextStart) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    connectStand(fixture);
    prepareAuto(fixture, validDroneParameters());
    setReadiness(fixture, application::session::ReadinessStatus::Dangerous);
    fixture.view.confirmationResponse = false;

    fixture.presenter.onStartPressed();
    fixture.presenter.onStartPressed();

    EXPECT_EQ(fixture.view.confirmationCount, 2);
    EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Idle);
}

TEST(ShellPresenterTest, Start_WhenStandDisconnected_DoesNotStart) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    fixture.state.setTestProtocolMode(domain::TestMode::Manual);

    fixture.presenter.onStartPressed();

    EXPECT_EQ(fixture.scheduler.startCount, 0);
    EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Idle);
    EXPECT_EQ(fixture.view.lastWarningTitle, "Стенд не подключён");
    ASSERT_FALSE(fixture.view.logs.empty());
    EXPECT_EQ(fixture.view.logs.back(), "Test execution start blocked: stand is not connected");
}

TEST(ShellPresenterTest, Start_WhenConnectionFailed_DoesNotStart) {
    ShellPresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);
    fixture.state.setStandConnectionStatus(domain::StandConnectionStatus::Error);
    fixture.state.setTestProtocolMode(domain::TestMode::Manual);

    fixture.presenter.onStartPressed();

    EXPECT_EQ(fixture.scheduler.startCount, 0);
    EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Idle);
    ASSERT_FALSE(fixture.view.logs.empty());
    EXPECT_EQ(fixture.view.logs.back(), "Test execution start blocked: stand is not connected");
}

TEST(ShellPresenterTest, Start_WhenConnected_AllowsStartForManualHybridAndAutomatic) {
    for (const auto mode : {domain::TestMode::Manual, domain::TestMode::Hybrid, domain::TestMode::Automatic}) {
        ShellPresenterFixture fixture{};
        fixture.presenter.attachView(fixture.view);
        connectStand(fixture);
        fixture.state.setTestProtocolMode(mode);
        fixture.state.setTestProtocolDroneParameters(validDroneParameters());
        fixture.state.setTestTimeSource(mode == domain::TestMode::Manual ? domain::TestTimeSource::FreeRun
                                                                         : domain::TestTimeSource::AutoCalculated);
        setReadiness(fixture, application::session::ReadinessStatus::Ok);

        fixture.presenter.onStartPressed();

        EXPECT_EQ(fixture.state.execution().testExecutionStatus, domain::TestExecutionStatus::Running);
    }
}
