#include "../../src/Presentation/ControlChartsTab/ControlChartsTabPresenter.hpp"

#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Application/UseCases/BuildControlPlotUseCase.hpp"
#include "../../src/Application/UseCases/EstimateTestDurationUseCase.hpp"
#include "../../src/Application/UseCases/SetControlChartsTabMinutesUseCase.hpp"
#include "../../src/Application/UseCases/SetWindImpactUseCase.hpp"
#include "../../src/Application/UseCases/UpdateTestProtocolUseCase.hpp"

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

class ControlChartsTabViewSpy final : public presentation::controlChartsTab::IControlChartsTabView {
  public:
    void setMinutes(int value) override {
        minutes = value;
    }

    void setMinutesInputEnabled(bool enabled) override {
        minutesInputEnabled = enabled;
    }

    void setReadinessCalculationEnabled(bool enabled) override {
        readinessCalculationEnabled = enabled;
    }

    void setTestProtocolMode(const std::string &mode) override {
        testProtocolMode = mode;
    }

    void setTestProtocolProgram(const std::string &program) override {
        testProtocolProgram = program;
    }

    void setBeaufort(double value) override {
        beaufort = value;
    }

    void setDirection(double value) override {
        direction = value;
    }

    void setAngleOfAttack(double value) override {
        angleOfAttack = value;
    }

    void refreshPlot() override {
        ++plotRefreshCount;
    }

    void showReadinessMessage(const std::string &message) override {
        readinessMessage = message;
    }

    void appendLog(const std::string &text) override {
        logMessages.push_back(text);
    }

    int minutes{0};
    bool minutesInputEnabled{false};
    bool readinessCalculationEnabled{false};
    std::string testProtocolMode{};
    std::string testProtocolProgram{};
    double beaufort{0.0};
    double direction{0.0};
    double angleOfAttack{0.0};
    int plotRefreshCount{0};
    std::string readinessMessage{};
    std::vector<std::string> logMessages{};
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

std::vector<domain::TestProtocolParameter> warningDroneParameters() {
    auto parameters = validDroneParameters();
    for (auto &parameter : parameters) {
        if (parameter.key == "frontal_area_m2" || parameter.key == "drag_coefficient") {
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

struct PresenterFixture {
    application::session::SessionState state{};
    application::useCases::SetControlChartsTabMinutesUseCase setMinutesUseCase{state};
    application::useCases::SetWindImpactUseCase setWindImpactUseCase{state};
    FunctionEngineStub functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::EstimateTestDurationUseCase estimateTestDurationUseCase{state};
    application::useCases::UpdateTestProtocolUseCase updateTestProtocolUseCase{state};
    presentation::controlChartsTab::ControlChartsTabPresenter presenter{
        presentation::controlChartsTab::ControlChartsTabPresenter::Dependencies{
            .state = state,
            .setControlChartsTabMinutesUseCase = setMinutesUseCase,
            .setWindImpactUseCase = setWindImpactUseCase,
            .buildControlPlotUseCase = buildControlPlotUseCase,
            .estimateTestDurationUseCase = estimateTestDurationUseCase,
            .updateTestProtocolUseCase = updateTestProtocolUseCase}};
    ControlChartsTabViewSpy view{};
};

} // namespace

TEST(ControlChartsTabPresenterTest, CalculateButtonEnabledOnlyForAutomaticAndHybridModes) {
    PresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);

    fixture.state.setTestProtocolMode(domain::TestMode::Manual);
    fixture.presenter.onViewReady();
    EXPECT_FALSE(fixture.view.readinessCalculationEnabled);

    fixture.state.setTestProtocolMode(domain::TestMode::Hybrid);
    fixture.presenter.onTimeSettingsChanged();
    EXPECT_TRUE(fixture.view.readinessCalculationEnabled);

    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.presenter.onTimeSettingsChanged();
    EXPECT_TRUE(fixture.view.readinessCalculationEnabled);
}

TEST(ControlChartsTabPresenterTest, ViewReadySyncsTestModeAndProgram) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Hybrid);
    fixture.state.setTestProtocolProgram(domain::TestProgram::MaximumWindLoad);
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onViewReady();

    EXPECT_EQ(fixture.view.testProtocolMode, "hybrid");
    EXPECT_EQ(fixture.view.testProtocolProgram, "test2");
}

TEST(ControlChartsTabPresenterTest, TestModeChangeUpdatesProtocolState) {
    PresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onTestProtocolModeChanged("automatic");

    EXPECT_EQ(fixture.state.protocol().testProtocol.testMode, domain::TestMode::Automatic);
    EXPECT_EQ(fixture.state.control().standControlMode, domain::StandControlMode::PresetScenario);
    EXPECT_EQ(fixture.state.protocol().testTimeSource, domain::TestTimeSource::AutoCalculated);
}

TEST(ControlChartsTabPresenterTest, TestProgramChangeUpdatesProtocolState) {
    PresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onTestProtocolProgramChanged("test3");

    EXPECT_EQ(fixture.state.protocol().testProtocol.testProgram, domain::TestProgram::WindLoadTemporalPerspective);
}

TEST(ControlChartsTabPresenterTest, CalculateReadinessRunsEstimateUseCase) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    fixture.state.setTestProtocolDroneParameters(validDroneParameters());
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onReadinessCalculationPressed();

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Ok);
    EXPECT_EQ(fixture.view.readinessMessage, "Расчёт готовности выполнен. Испытание допустимо.");
}

TEST(ControlChartsTabPresenterTest, WarningStatusMapsToWarningMessage) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    fixture.state.setTestProtocolDroneParameters(warningDroneParameters());
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onReadinessCalculationPressed();

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Warning);
    EXPECT_EQ(fixture.view.readinessMessage, "Расчёт готовности выполнен. Есть предупреждения.");
}

TEST(ControlChartsTabPresenterTest, FailedStatusMapsToDangerMessage) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    fixture.state.setTestProtocolDroneParameters(invalidDroneParameters());
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onReadinessCalculationPressed();

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Failed);
    EXPECT_EQ(fixture.view.readinessMessage, "Расчёт готовности невозможен. Испытание потенциально опасно.");
}

TEST(ControlChartsTabPresenterTest, ReadinessStatusMessagesAreMapped) {
    using application::session::ReadinessStatus;
    using presentation::controlChartsTab::ControlChartsTabPresenter;

    EXPECT_EQ(ControlChartsTabPresenter::messageForReadinessStatus(ReadinessStatus::Ok),
              "Расчёт готовности выполнен. Испытание допустимо.");
    EXPECT_EQ(ControlChartsTabPresenter::messageForReadinessStatus(ReadinessStatus::Warning),
              "Расчёт готовности выполнен. Есть предупреждения.");
    EXPECT_EQ(ControlChartsTabPresenter::messageForReadinessStatus(ReadinessStatus::Dangerous),
              "Испытание потенциально опасно.");
    EXPECT_EQ(ControlChartsTabPresenter::messageForReadinessStatus(ReadinessStatus::Failed),
              "Расчёт готовности невозможен. Испытание потенциально опасно.");
}
