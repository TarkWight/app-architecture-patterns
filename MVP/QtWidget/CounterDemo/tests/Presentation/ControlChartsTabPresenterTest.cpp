#include "../../src/Presentation/ControlChartsTab/ControlChartsTabPresenter.hpp"

#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Application/UseCases/BuildControlPlotUseCase.hpp"
#include "../../src/Application/UseCases/SetControlChartsTabMinutesUseCase.hpp"
#include "../../src/Application/UseCases/SetUseAngleOfAttackModelUseCase.hpp"
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

    void setOperatorDurationVisible(bool visible) override {
        operatorDurationVisible = visible;
    }

    void setEstimatedDurationVisible(bool visible) override {
        estimatedDurationVisible = visible;
    }

    void setEstimatedDurationText(const std::string &text) override {
        estimatedDurationText = text;
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

    void setUseAngleOfAttackModel(bool enabled) override {
        useAngleOfAttackModel = enabled;
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
    bool operatorDurationVisible{true};
    bool estimatedDurationVisible{false};
    std::string estimatedDurationText{};
    std::string testProtocolMode{};
    std::string testProtocolProgram{};
    double beaufort{0.0};
    double direction{0.0};
    double angleOfAttack{0.0};
    bool useAngleOfAttackModel{false};
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

std::vector<domain::TestProtocolParameter> invalidDroneParameters() {
    return {
        {"uav_model", "Модель БАС", "BAS-1"},
        {"battery_capacity_mah", "Емкость", "0"},
    };
}

bool contains(const std::string &text, const std::string &fragment) {
    return text.find(fragment) != std::string::npos;
}

struct PresenterFixture {
    application::session::SessionState state{};
    application::useCases::SetControlChartsTabMinutesUseCase setMinutesUseCase{state};
    application::useCases::SetWindImpactUseCase setWindImpactUseCase{state};
    application::useCases::SetUseAngleOfAttackModelUseCase setUseAngleOfAttackModelUseCase{state};
    FunctionEngineStub functionEngine{};
    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{state, functionEngine};
    application::useCases::UpdateTestProtocolUseCase updateTestProtocolUseCase{state};
    presentation::controlChartsTab::ControlChartsTabPresenter presenter{
        presentation::controlChartsTab::ControlChartsTabPresenter::Dependencies{
            .state = state,
            .setControlChartsTabMinutesUseCase = setMinutesUseCase,
            .setWindImpactUseCase = setWindImpactUseCase,
            .setUseAngleOfAttackModelUseCase = setUseAngleOfAttackModelUseCase,
            .buildControlPlotUseCase = buildControlPlotUseCase,
            .updateTestProtocolUseCase = updateTestProtocolUseCase}};
    ControlChartsTabViewSpy view{};
};

} // namespace

TEST(ControlChartsTabPresenterTest, ViewReadySyncsTestModeAndProgram) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Hybrid);
    fixture.state.setTestProtocolProgram(domain::TestProgram::MaximumWindLoad);
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onViewReady();

    EXPECT_EQ(fixture.view.testProtocolMode, "hybrid");
    EXPECT_EQ(fixture.view.testProtocolProgram, "test2");
    EXPECT_EQ(fixture.view.plotRefreshCount, 0);
}

TEST(ControlChartsTabPresenterTest, ViewReadySyncsAngleOfAttackModelFlag) {
    PresenterFixture fixture{};
    fixture.state.setUseAngleOfAttackModel(true);
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onViewReady();

    EXPECT_TRUE(fixture.view.useAngleOfAttackModel);
}

TEST(ControlChartsTabPresenterTest, TestModeChangeUpdatesProtocolStateWithoutRebuildingPlot) {
    PresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onTestProtocolModeChanged("automatic");

    EXPECT_EQ(fixture.state.protocol().testProtocol.testMode, domain::TestMode::Automatic);
    EXPECT_EQ(fixture.state.control().standControlMode, domain::StandControlMode::PresetScenario);
    EXPECT_EQ(fixture.state.protocol().testTimeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(fixture.view.plotRefreshCount, 0);
}

TEST(ControlChartsTabPresenterTest, AngleOfAttackModelChangeUpdatesStateWithoutRebuildingPlot) {
    PresenterFixture fixture{};
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onUseAngleOfAttackModelChanged(true);

    EXPECT_TRUE(fixture.state.control().useAngleOfAttackModel);
    EXPECT_EQ(fixture.view.plotRefreshCount, 0);
}

TEST(ControlChartsTabPresenterTest, AngleOfAttackModelChangeResetsReadiness) {
    PresenterFixture fixture{};
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(37);
    fixture.state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    ASSERT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Ok);
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onUseAngleOfAttackModelChanged(true);

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Unknown);
    EXPECT_TRUE(fixture.view.readinessMessage.empty());
}

TEST(ControlChartsTabPresenterTest, TestProgramChangeUpdatesProtocolStateAndFormulaWithoutRebuildingPlot) {
    PresenterFixture fixture{};
    fixture.state.setFunctionExpression("operator_custom_formula");
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onTestProtocolProgramChanged("test3");

    EXPECT_EQ(fixture.state.protocol().testProtocol.testProgram, domain::TestProgram::WindLoadTemporalPerspective);
    EXPECT_EQ(fixture.state.control().functionExpression.value, "sin(x) * (6.9 * sin(10 * x))");
    EXPECT_EQ(fixture.view.plotRefreshCount, 0);
}

TEST(ControlChartsTabPresenterTest, CustomTestProgramKeepsOperatorFormulaEditableSource) {
    PresenterFixture fixture{};
    fixture.state.setFunctionExpression("operator_custom_formula");
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onTestProtocolProgramChanged("custom");

    EXPECT_EQ(fixture.state.protocol().testProtocol.testProgram, domain::TestProgram::Custom);
    EXPECT_EQ(fixture.state.control().functionExpression.value, "operator_custom_formula");
    EXPECT_EQ(fixture.view.plotRefreshCount, 0);
}

TEST(ControlChartsTabPresenterTest, TestProgramChangeResetsReadiness) {
    PresenterFixture fixture{};
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(37);
    fixture.state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    ASSERT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Ok);
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onTestProtocolProgramChanged("test2");

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Unknown);
}

TEST(ControlChartsTabPresenterTest, CalculationResultDisplaysOkReadinessMessage) {
    PresenterFixture fixture{};
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(37);
    fixture.state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onCalculationResultChanged();

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Ok);
    EXPECT_TRUE(contains(fixture.view.readinessMessage, "Расчёт готовности выполнен. Испытание допустимо."));
    EXPECT_EQ(fixture.view.plotRefreshCount, 1);
}

TEST(ControlChartsTabPresenterTest, WarningStatusMapsToWarningMessage) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(37);
    result.warnings.push_back(
        domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::FrontalAreaFallbackUsed});
    result.warnings.push_back(
        domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::DragCoefficientFallbackUsed});
    fixture.state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onCalculationResultChanged();

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Warning);
    EXPECT_TRUE(contains(fixture.view.readinessMessage, "Расчёт готовности выполнен. Есть предупреждения."));
    EXPECT_TRUE(contains(fixture.view.readinessMessage, "Фронтальная площадь"));
    EXPECT_TRUE(contains(fixture.view.readinessMessage, "Коэффициент сопротивления"));
}

TEST(ControlChartsTabPresenterTest, FailedStatusMapsToDangerMessage) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    domain::EstimatedTestDurationResult result{};
    result.errors.push_back(
        domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::BatteryCapacityMissing});
    fixture.state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onCalculationResultChanged();

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Failed);
    EXPECT_TRUE(
        contains(fixture.view.readinessMessage, "Расчёт готовности невозможен. Испытание потенциально опасно."));
    EXPECT_TRUE(contains(fixture.view.readinessMessage, "Ёмкость АКБ"));
}

TEST(ControlChartsTabPresenterTest, AutoCalculatedShowsEstimatedDurationNotControlChartsMinutes) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    fixture.state.setControlChartsTabMinutes(domain::DurationMinutes::required(20));
    fixture.state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(37));
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(37);
    fixture.state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onViewReady();

    EXPECT_FALSE(fixture.view.operatorDurationVisible);
    EXPECT_TRUE(fixture.view.estimatedDurationVisible);
    EXPECT_EQ(fixture.view.estimatedDurationText, "37 мин");
}

TEST(ControlChartsTabPresenterTest, OperatorDefinedShowsControlChartsMinutes) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Hybrid);
    fixture.state.setTestTimeSource(domain::TestTimeSource::OperatorDefined);
    fixture.state.setControlChartsTabMinutes(domain::DurationMinutes::required(42));
    fixture.state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(7));
    fixture.presenter.attachView(fixture.view);

    fixture.presenter.onViewReady();

    EXPECT_TRUE(fixture.view.operatorDurationVisible);
    EXPECT_FALSE(fixture.view.estimatedDurationVisible);
    EXPECT_EQ(fixture.view.minutes, 42);
}

TEST(ControlChartsTabPresenterTest, SuccessfulReadinessCalculationUpdatesEstimatedDurationDisplay) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    fixture.state.setControlChartsTabMinutes(domain::DurationMinutes::required(20));
    fixture.state.setTestProtocolDroneParameters(validDroneParameters());
    fixture.presenter.attachView(fixture.view);

    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(37);
    fixture.state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(37));
    fixture.state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    fixture.presenter.onCalculationResultChanged();

    EXPECT_TRUE(fixture.view.estimatedDurationVisible);
    EXPECT_EQ(fixture.view.estimatedDurationText,
              std::to_string(fixture.state.protocol().estimatedTestDuration.value()) + " мин");
}

TEST(ControlChartsTabPresenterTest, FailedReadinessDoesNotPresentFallbackDurationAsSuccessful) {
    PresenterFixture fixture{};
    fixture.state.setTestProtocolMode(domain::TestMode::Automatic);
    fixture.state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    fixture.state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(20));
    fixture.state.setTestProtocolDroneParameters(invalidDroneParameters());
    fixture.presenter.attachView(fixture.view);

    domain::EstimatedTestDurationResult result{};
    result.errors.push_back(
        domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::BatteryCapacityMissing});
    fixture.state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    fixture.presenter.onCalculationResultChanged();

    EXPECT_EQ(fixture.state.readiness().status, application::session::ReadinessStatus::Failed);
    EXPECT_TRUE(fixture.view.estimatedDurationVisible);
    EXPECT_EQ(fixture.view.estimatedDurationText, "Расчёт не выполнен");
}
