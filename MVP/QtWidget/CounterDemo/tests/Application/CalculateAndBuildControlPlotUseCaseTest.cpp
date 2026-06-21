#include "../../src/Application/UseCases/CalculateAndBuildControlPlotUseCase.hpp"

#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Application/Services/UavSpecificationMapper.hpp"
#include "../../src/Domain/TestDurationEstimator.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

namespace {

class DelayedPeakFunctionEngine final : public application::ports::IFunctionEngine {
  public:
    double eval(const std::string & /*expr*/, double x) const override {
        return x >= 30.0 ? 6.0 : 1.0;
    }
};

class TrackingFunctionEngine final : public application::ports::IFunctionEngine {
  public:
    double eval(const std::string & /*expr*/, double x) const override {
        maxX = std::max(maxX, x);
        return x >= 5.0 ? 7.0 : 2.0;
    }

    mutable double maxX{0.0};
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

domain::DurationMinutes expectedDuration(const domain::TestProtocol &protocol, const domain::WindImpact &impact) {
    const auto uav = application::services::UavSpecificationMapper{}.map(protocol);
    const auto result = domain::TestDurationEstimator::estimate(domain::TestDurationEstimationContext{
        .uav = *uav,
        .impact = impact,
    });
    return *result.duration;
}

TEST(CalculateAndBuildControlPlotUseCaseTest, AutoCalculatedAnalyzesFunctionBeforeFinalPlot) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));
    state.setWindImpact(domain::makeWindImpact(1.0, 0.0, 0.0));
    state.setTestProtocolDroneParameters(validDroneParameters());
    const DelayedPeakFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase buildUseCase{state, engine};
    application::useCases::CalculateAndBuildControlPlotUseCase useCase{state, engine, buildUseCase};

    useCase.execute();

    EXPECT_TRUE(state.readiness().calculatedForWorstCaseScenario);
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.beaufort.value(), 6.0);
}

TEST(CalculateAndBuildControlPlotUseCaseTest, WhenEstimatorSucceedsBuildsPlotUsingEstimatedDuration) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));
    state.setWindImpact(domain::makeWindImpact(1.0, 0.0, 0.0));
    state.setTestProtocolDroneParameters(validDroneParameters());
    const DelayedPeakFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase buildUseCase{state, engine};
    application::useCases::CalculateAndBuildControlPlotUseCase useCase{state, engine, buildUseCase};

    useCase.execute();

    const auto expected = expectedDuration(state.protocol().testProtocol, state.readiness().calculatedForImpact);
    EXPECT_EQ(state.protocol().estimatedTestDuration.value(), expected.value());
    EXPECT_EQ(state.control().controlProfile.duration.value(), expected.value());
    EXPECT_EQ(state.control().controlPlot.x.max, static_cast<double>(expected.value()));
}

TEST(CalculateAndBuildControlPlotUseCaseTest, WhenEstimatorFailsDoesNotOverwriteFinalPlotAsSuccessful) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolDroneParameters(invalidDroneParameters());
    application::dto::PlotModel previousPlot{};
    previousPlot.title = "previous";
    state.setControlPlot(previousPlot);
    domain::WindControlProfile previousProfile{};
    previousProfile.duration = domain::DurationMinutes::required(9);
    state.setControlProfile(previousProfile);
    const DelayedPeakFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase buildUseCase{state, engine};
    application::useCases::CalculateAndBuildControlPlotUseCase useCase{state, engine, buildUseCase};

    useCase.execute();

    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Failed);
    EXPECT_EQ(state.control().controlPlot.title, "previous");
    EXPECT_EQ(state.control().controlProfile.duration.value(), 9);
}

TEST(CalculateAndBuildControlPlotUseCaseTest, HybridOperatorDefinedUsesOperatorDurationAsAnalysisWindow) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Hybrid);
    state.setTestTimeSource(domain::TestTimeSource::OperatorDefined);
    state.setOperatorTestDurationMinutes(domain::DurationMinutes::required(3));
    state.setTestProtocolDroneParameters(validDroneParameters());
    const TrackingFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase buildUseCase{state, engine};
    application::useCases::CalculateAndBuildControlPlotUseCase useCase{state, engine, buildUseCase};

    useCase.execute();

    EXPECT_LT(engine.maxX, 5.0);
    EXPECT_EQ(state.control().controlProfile.duration.value(), 3);
}

TEST(CalculateAndBuildControlPlotUseCaseTest, ManualDoesNotRunReadinessEstimator) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Manual);
    state.setTestTimeSource(domain::TestTimeSource::FreeRun);
    state.setTestProtocolDroneParameters(invalidDroneParameters());
    const DelayedPeakFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase buildUseCase{state, engine};
    application::useCases::CalculateAndBuildControlPlotUseCase useCase{state, engine, buildUseCase};

    useCase.execute();

    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Unknown);
    EXPECT_TRUE(state.control().controlProfile.samples.empty());
}

} // namespace
