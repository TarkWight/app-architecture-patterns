#include "../../src/Application/Session/SessionState.hpp"

#include <gtest/gtest.h>

namespace {

TEST(SessionStateTest, InitializesTelemetryPlotWithVisibleCoordinateGrid) {
    const application::session::SessionState state{};
    const auto &plot = state.get().telemetry.telemetryPlot;

    EXPECT_EQ(plot.title, "Телеметрия");
    EXPECT_DOUBLE_EQ(plot.x.min, 0.0);
    EXPECT_DOUBLE_EQ(plot.x.max, 60.0);
    EXPECT_DOUBLE_EQ(plot.x.step, 10.0);
    EXPECT_EQ(plot.x.label, "секунды");
    EXPECT_EQ(plot.x.labelPrecision, 0);
    EXPECT_DOUBLE_EQ(plot.y.min, 0.0);
    EXPECT_DOUBLE_EQ(plot.y.max, 360.0);
    EXPECT_DOUBLE_EQ(plot.y.step, 45.0);
    EXPECT_EQ(plot.y.label, "градусы");
}

TEST(SessionStateTest, ExposesNarrowStateGroups) {
    application::session::SessionState state{};

    state.setTestExecutionStatus(domain::TestExecutionStatus::Running);
    state.setStandConnectionStatus(domain::StandConnectionStatus::Connected);
    state.setTelemetryStatus(domain::TelemetryStatus::Valid);
    state.setWindImpact(domain::makeWindImpact(3.0, 90.0, 15.0));
    state.setTestProtocolTitle("Protocol");

    EXPECT_EQ(state.execution().testExecutionStatus, domain::TestExecutionStatus::Running);
    EXPECT_EQ(state.connection().standConnectionStatus, domain::StandConnectionStatus::Connected);
    EXPECT_EQ(state.telemetry().telemetryStatus, domain::TelemetryStatus::Valid);
    EXPECT_DOUBLE_EQ(state.control().windImpact.beaufort.value(), 3.0);
    EXPECT_EQ(state.protocol().testProtocol.title, "Protocol");
}

TEST(SessionStateTest, StoresDangerousReadinessWhenDurationHasErrors) {
    application::session::SessionState state{};
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(12);
    result.errors.push_back(
        domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::RequiredThrustExceedsAvailable});
    result.values.estimatedDurationMinutes = 12.0;
    const auto impact = domain::makeWindImpact(4.0, 90.0, 10.0);

    state.setReadinessFromEstimationResult(result, impact);

    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Dangerous);
    ASSERT_EQ(state.readiness().errors.size(), 1U);
    EXPECT_EQ(state.readiness().errors.front().code,
              domain::TestDurationDiagnosticCode::RequiredThrustExceedsAvailable);
    EXPECT_DOUBLE_EQ(state.readiness().values.estimatedDurationMinutes, 12.0);
    EXPECT_TRUE(state.readiness().hasCalculatedForImpact);
    EXPECT_DOUBLE_EQ(state.readiness().calculatedForImpact.direction.degrees(), 90.0);
}

TEST(SessionStateTest, ResetsReadinessWhenEstimationInputsChange) {
    application::session::SessionState state{};
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(12);
    state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    ASSERT_EQ(state.readiness().status, application::session::ReadinessStatus::Ok);

    state.setTargetStandImpact(domain::makeWindImpact(2.0, 90.0, 0.0));

    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Unknown);
    EXPECT_FALSE(state.readiness().hasCalculatedForImpact);
}

TEST(SessionStateTest, ResetsReadinessWhenHybridOverrideChanges) {
    application::session::SessionState state{};
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(12);
    state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    ASSERT_EQ(state.readiness().status, application::session::ReadinessStatus::Ok);

    state.setHybridBeaufortOverride(domain::HybridBeaufortOverridePolicy::startOverride(
        domain::Beaufort::from(1.0), domain::Beaufort::from(6.0), domain::ElapsedSeconds::from(0)));

    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Unknown);
    EXPECT_FALSE(state.readiness().hasCalculatedForImpact);
}

TEST(SessionStateTest, ResetsReadinessWhenHybridOverrideIsCleared) {
    application::session::SessionState state{};
    state.setHybridBeaufortOverride(domain::HybridBeaufortOverridePolicy::startOverride(
        domain::Beaufort::from(1.0), domain::Beaufort::from(6.0), domain::ElapsedSeconds::from(0)));
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(12);
    state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    ASSERT_EQ(state.readiness().status, application::session::ReadinessStatus::Ok);

    state.clearHybridBeaufortOverride();

    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Unknown);
    EXPECT_FALSE(state.readiness().hasCalculatedForImpact);
}

TEST(SessionStateTest, RuntimeTargetStandImpactDoesNotResetReadiness) {
    application::session::SessionState state{};
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(12);
    state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    ASSERT_EQ(state.readiness().status, application::session::ReadinessStatus::Ok);

    state.setRuntimeTargetStandImpact(domain::makeWindImpact(2.0, 90.0, 0.0));

    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Ok);
    EXPECT_TRUE(state.readiness().hasCalculatedForImpact);
}

TEST(SessionStateTest, ResetsReadinessWhenAngleOfAttackModelFlagChanges) {
    application::session::SessionState state{};
    domain::EstimatedTestDurationResult result{};
    result.duration = domain::DurationMinutes::required(12);
    state.setReadinessFromEstimationResult(result, domain::makeWindImpact(1.0, 0.0, 0.0));
    ASSERT_EQ(state.readiness().status, application::session::ReadinessStatus::Ok);

    state.setUseAngleOfAttackModel(true);

    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Unknown);
    EXPECT_FALSE(state.readiness().hasCalculatedForImpact);
}

} // namespace
