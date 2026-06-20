#include "../../src/Application/UseCases/EstimateTestDurationUseCase.hpp"

#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Application/Services/UavSpecificationMapper.hpp"
#include "../../src/Domain/TestDurationEstimator.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

namespace {

class PeakFunctionEngine final : public application::ports::IFunctionEngine {
  public:
    explicit PeakFunctionEngine(double peak) : peak(peak) {
    }

    double eval(const std::string & /*expr*/, double /*x*/) const override {
        return peak;
    }

  private:
    double peak{0.0};
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

std::vector<domain::TestProtocolParameter> validDroneParametersWithFallbackWarnings() {
    auto parameters = validDroneParameters();
    for (auto &parameter : parameters) {
        if (parameter.key == "frontal_area_m2" || parameter.key == "drag_coefficient") {
            parameter.value = "";
        }
    }
    return parameters;
}

domain::DurationMinutes expectedDuration(const domain::TestProtocol &protocol, const domain::WindImpact &impact) {
    const auto uav = application::services::UavSpecificationMapper{}.map(protocol);
    const auto result = domain::TestDurationEstimator::estimate(domain::TestDurationEstimationContext{
        .uav = *uav,
        .impact = impact,
    });
    return *result.duration;
}

bool hasDiagnostic(const std::vector<domain::TestDurationDiagnostic> &diagnostics,
                   domain::TestDurationDiagnosticCode code) {
    return std::ranges::any_of(diagnostics, [code](const auto &diagnostic) { return diagnostic.code == code; });
}

domain::WindControlProfile profileWithBeaufort(std::initializer_list<double> beaufortValues) {
    domain::WindControlProfile profile{};
    double seconds = 0.0;
    for (const auto beaufort : beaufortValues) {
        profile.samples.push_back(domain::WindControlSample{
            .time = domain::WindControlSampleTime::fromSeconds(seconds),
            .beaufort = domain::Beaufort::from(beaufort),
        });
        seconds += profile.sampleIntervalSeconds;
    }
    return profile;
}

TEST(EstimateTestDurationUseCaseTest, WhenEstimatorReturnsDuration_UpdatesEstimatedDuration) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolDroneParameters(validDroneParameters());
    const auto impact = domain::makeWindImpact(4.0, 0.0, 10.0);
    state.setWindImpact(impact);

    application::useCases::EstimateTestDurationUseCase useCase{state};

    const auto result = useCase.executeForAutoCalculated();

    ASSERT_TRUE(result.duration.has_value());
    EXPECT_EQ(state.protocol().estimatedTestDuration.value(),
              expectedDuration(state.protocol().testProtocol, impact).value());
    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Ok);
    EXPECT_TRUE(state.readiness().warnings.empty());
    EXPECT_TRUE(state.readiness().errors.empty());
    EXPECT_TRUE(state.readiness().hasCalculatedForImpact);
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.beaufort.value(), impact.beaufort.value());
}

TEST(EstimateTestDurationUseCaseTest, WhenEstimatorFails_DoesNotOverwriteEstimatedDuration) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(37));
    state.setTestProtocolDroneParameters(invalidDroneParameters());

    application::useCases::EstimateTestDurationUseCase useCase{state};

    const auto result = useCase.executeForAutoCalculated();

    EXPECT_FALSE(result.duration.has_value());
    EXPECT_EQ(state.protocol().estimatedTestDuration.value(), 37);
    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Failed);
    EXPECT_FALSE(state.readiness().errors.empty());
}

TEST(EstimateTestDurationUseCaseTest, WhenEstimatorReturnsWarnings_StoresWarningReadiness) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolDroneParameters(validDroneParametersWithFallbackWarnings());

    application::useCases::EstimateTestDurationUseCase useCase{state};

    const auto result = useCase.executeForAutoCalculated();

    ASSERT_TRUE(result.duration.has_value());
    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Warning);
    EXPECT_TRUE(hasDiagnostic(state.readiness().warnings, domain::TestDurationDiagnosticCode::FrontalAreaFallbackUsed));
    EXPECT_TRUE(
        hasDiagnostic(state.readiness().warnings, domain::TestDurationDiagnosticCode::DragCoefficientFallbackUsed));
    EXPECT_TRUE(state.readiness().errors.empty());
}

TEST(EstimateTestDurationUseCaseTest, NextEstimateReplacesPreviousDiagnostics) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolDroneParameters(validDroneParametersWithFallbackWarnings());
    application::useCases::EstimateTestDurationUseCase useCase{state};

    ASSERT_TRUE(useCase.executeForAutoCalculated().duration.has_value());
    ASSERT_EQ(state.readiness().status, application::session::ReadinessStatus::Warning);

    state.setTestProtocolDroneParameters(validDroneParameters());
    ASSERT_TRUE(useCase.executeForAutoCalculated().duration.has_value());

    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Ok);
    EXPECT_TRUE(state.readiness().warnings.empty());
    EXPECT_TRUE(state.readiness().errors.empty());
}

TEST(EstimateTestDurationUseCaseTest, FailedEstimateDoesNotOverwritePreviousEstimatedDuration) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(42));
    state.setTestProtocolDroneParameters(invalidDroneParameters());

    application::useCases::EstimateTestDurationUseCase useCase{state};

    const auto result = useCase.executeForAutoCalculated();

    EXPECT_FALSE(result.duration.has_value());
    EXPECT_EQ(state.protocol().estimatedTestDuration.value(), 42);
    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Failed);
}

TEST(EstimateTestDurationUseCaseTest, ManualUsesCurrentImpact) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolMode(domain::TestMode::Manual);
    state.setTestProtocolDroneParameters(validDroneParameters());
    const auto currentImpact = domain::makeWindImpact(2.0, 0.0, -15.0);
    state.setWindImpact(currentImpact);
    state.setControlProfile(profileWithBeaufort({7.0}));

    application::useCases::EstimateTestDurationUseCase useCase{state};

    ASSERT_TRUE(useCase.executeForAutoCalculated().duration.has_value());

    EXPECT_FALSE(state.readiness().calculatedForWorstCaseScenario);
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.beaufort.value(), currentImpact.beaufort.value());
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.angleOfAttack.degrees(),
                     currentImpact.angleOfAttack.degrees());
}

TEST(EstimateTestDurationUseCaseTest, HybridUsesProfilePeakBeaufort) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolMode(domain::TestMode::Hybrid);
    state.setTestProtocolDroneParameters(validDroneParameters());
    state.setWindImpact(domain::makeWindImpact(1.0, 0.0, 0.0));
    state.setControlProfile(profileWithBeaufort({1.0, 6.0, 3.0}));

    application::useCases::EstimateTestDurationUseCase useCase{state};

    ASSERT_TRUE(useCase.executeForAutoCalculated().duration.has_value());

    EXPECT_TRUE(state.readiness().calculatedForWorstCaseScenario);
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.beaufort.value(), 6.0);
}

TEST(EstimateTestDurationUseCaseTest, HybridReadinessUsesHybridOperatorDirectionInCalculatedImpact) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolMode(domain::TestMode::Hybrid);
    state.setTestProtocolDroneParameters(validDroneParameters());
    state.setWindImpact(domain::makeWindImpact(1.0, 0.0, 0.0));
    state.setControlProfile(profileWithBeaufort({4.0}));
    state.setHybridOperatorDirection(domain::WindDirection::from(270.0));

    application::useCases::EstimateTestDurationUseCase useCase{state};

    ASSERT_TRUE(useCase.executeForAutoCalculated().duration.has_value());

    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.direction.degrees(), 270.0);
}

TEST(EstimateTestDurationUseCaseTest, HybridReadinessUsesHybridOperatorAngleOfAttack) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolMode(domain::TestMode::Hybrid);
    state.setTestProtocolDroneParameters(validDroneParameters());
    state.setWindImpact(domain::makeWindImpact(1.0, 0.0, 0.0));
    state.setControlProfile(profileWithBeaufort({4.0}));
    state.setHybridOperatorAngleOfAttack(domain::AngleOfAttack::from(-45.0));

    application::useCases::EstimateTestDurationUseCase useCase{state};

    ASSERT_TRUE(useCase.executeForAutoCalculated().duration.has_value());

    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.angleOfAttack.degrees(), -45.0);
    EXPECT_GT(state.readiness().values.anglePenalty, 1.0);
}

TEST(EstimateTestDurationUseCaseTest, HybridReadinessConsidersActiveOverrideOperatorBeaufort) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolMode(domain::TestMode::Hybrid);
    state.setTestProtocolDroneParameters(validDroneParameters());
    state.setWindImpact(domain::makeWindImpact(1.0, 0.0, 0.0));
    state.setControlProfile(profileWithBeaufort({1.0, 3.0}));
    state.setHybridBeaufortOverride(domain::HybridBeaufortOverridePolicy::startOverride(
        domain::Beaufort::from(3.0), domain::Beaufort::from(6.0), domain::ElapsedSeconds::from(0)));

    application::useCases::EstimateTestDurationUseCase useCase{state};

    ASSERT_TRUE(useCase.executeForAutoCalculated().duration.has_value());

    EXPECT_TRUE(state.readiness().calculatedForWorstCaseScenario);
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.beaufort.value(), 6.0);
}

TEST(EstimateTestDurationUseCaseTest, OverrideBeaufortAboveProfilePeakIncreasesReadinessRisk) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolMode(domain::TestMode::Hybrid);
    state.setTestProtocolDroneParameters(validDroneParameters());
    state.setWindImpact(domain::makeWindImpact(1.0, 0.0, 0.0));
    state.setControlProfile(profileWithBeaufort({3.0}));
    state.setHybridBeaufortOverride(domain::HybridBeaufortOverridePolicy::startOverride(
        domain::Beaufort::from(3.0), domain::Beaufort::from(7.0), domain::ElapsedSeconds::from(0)));

    application::useCases::EstimateTestDurationUseCase useCase{state};

    const auto result = useCase.executeForAutoCalculated();

    ASSERT_TRUE(result.duration.has_value());
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.beaufort.value(), 7.0);
    EXPECT_LT(result.duration->value(),
              expectedDuration(state.protocol().testProtocol, domain::makeWindImpact(3.0, 0.0, 0.0)).value());
}

TEST(EstimateTestDurationUseCaseTest, AutomaticUsesPreviewProfilePeakBeaufortWhenProfileIsNotBuilt) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestProtocolDroneParameters(validDroneParameters());
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));
    state.setWindImpact(domain::makeWindImpact(1.0, 0.0, 0.0));
    state.setFunctionExpression("peak");
    PeakFunctionEngine engine{5.0};

    application::useCases::EstimateTestDurationUseCase useCase{state, engine};

    ASSERT_TRUE(useCase.executeForAutoCalculated().duration.has_value());

    EXPECT_TRUE(state.readiness().calculatedForWorstCaseScenario);
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.beaufort.value(), 5.0);
}

TEST(EstimateTestDurationUseCaseTest, CurrentImpactBelowProfilePeakDoesNotLowerRisk) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestProtocolDroneParameters(validDroneParameters());
    state.setWindImpact(domain::makeWindImpact(1.0, 0.0, 0.0));
    state.setControlProfile(profileWithBeaufort({7.0}));

    application::useCases::EstimateTestDurationUseCase useCase{state};

    const auto result = useCase.executeForAutoCalculated();

    ASSERT_TRUE(result.duration.has_value());
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.beaufort.value(), 7.0);
    EXPECT_LT(result.duration->value(),
              expectedDuration(state.protocol().testProtocol, domain::makeWindImpact(1.0, 0.0, 0.0)).value());
}

TEST(EstimateTestDurationUseCaseTest, EmptyProfileFallsBackToCurrentImpact) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestProtocolDroneParameters(validDroneParameters());
    const auto currentImpact = domain::makeWindImpact(3.0, 0.0, 0.0);
    state.setWindImpact(currentImpact);

    application::useCases::EstimateTestDurationUseCase useCase{state};

    ASSERT_TRUE(useCase.executeForAutoCalculated().duration.has_value());

    EXPECT_FALSE(state.readiness().calculatedForWorstCaseScenario);
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.beaufort.value(), currentImpact.beaufort.value());
}

TEST(EstimateTestDurationUseCaseTest, WorstCaseAngleUsesCurrentAbsoluteAngle) {
    application::session::SessionState state{};
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setTestProtocolMode(domain::TestMode::Hybrid);
    state.setTestProtocolDroneParameters(validDroneParameters());
    state.setWindImpact(domain::makeWindImpact(1.0, 0.0, -45.0));
    state.setControlProfile(profileWithBeaufort({4.0}));
    state.setHybridOperatorAngleOfAttack(domain::AngleOfAttack::from(-45.0));

    application::useCases::EstimateTestDurationUseCase useCase{state};

    ASSERT_TRUE(useCase.executeForAutoCalculated().duration.has_value());

    EXPECT_TRUE(state.readiness().calculatedForWorstCaseScenario);
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.angleOfAttack.degrees(), -45.0);
    EXPECT_GT(state.readiness().values.anglePenalty, 1.0);
}

} // namespace
