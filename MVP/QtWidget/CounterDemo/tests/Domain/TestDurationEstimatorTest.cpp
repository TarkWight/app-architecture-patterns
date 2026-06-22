#include "../../src/Domain/TestDurationEstimator.hpp"

#include <gtest/gtest.h>

#include <algorithm>

namespace {

domain::UavSpecification validUav() {
    domain::UavSpecification uav{};
    uav.frame.totalWeightKg = 6.0;
    uav.frame.flightPayload = 1.0;
    uav.frame.frontalAreaM2 = 0.25;
    uav.frame.dragCoefficient = 1.0;
    uav.frame.equipmentCurrent = 1.0;
    uav.battery.capacityMah = 18000;
    uav.battery.cellCount = 6;
    uav.battery.cellVoltage = 3.8;
    uav.battery.dischargeRateC = 6.0;
    uav.battery.weight = 2.4;
    uav.motor.count = 4;
    uav.motor.maxThrustKg = 3.0;
    uav.motor.peakCurrentA = 30.0;
    uav.motor.hoverCurrentA = 8.0;
    uav.motor.weight = 0.45;
    return uav;
}

bool hasDiagnostic(const std::vector<domain::TestDurationDiagnostic> &diagnostics,
                   domain::TestDurationDiagnosticCode code) {
    return std::ranges::any_of(diagnostics, [code](const auto &diagnostic) { return diagnostic.code == code; });
}

domain::EstimatedTestDurationResult estimate(domain::UavSpecification uav, domain::WindImpact impact) {
    return domain::TestDurationEstimator::estimate(domain::TestDurationEstimationContext{.uav = uav, .impact = impact});
}

TEST(TestDurationEstimatorTest, BatteryEnergy_WhenValidBattery_ReturnsWh) {
    const auto uav = validUav();

    const auto energy = domain::TestDurationEstimator::batteryEnergyWh(uav);

    EXPECT_NEAR(energy, 18.0 * 6.0 * 3.8 * 0.8, 0.000001);
}

TEST(TestDurationEstimatorTest, Estimator_WhenBeaufortIncreases_DurationDecreases) {
    const auto calm = estimate(validUav(), domain::makeWindImpact(0.0, 0.0, 0.0));
    const auto windy = estimate(validUav(), domain::makeWindImpact(7.0, 0.0, 0.0));

    ASSERT_TRUE(calm.duration.has_value());
    ASSERT_TRUE(windy.duration.has_value());
    EXPECT_GT(calm.values.estimatedDurationMinutes, windy.values.estimatedDurationMinutes);
}

TEST(TestDurationEstimatorTest, Estimator_WhenMassIncreases_DurationDecreases) {
    auto light = validUav();
    auto heavy = validUav();
    heavy.frame.totalWeightKg = 10.0;

    const auto lightResult = estimate(light, domain::makeWindImpact(3.0, 0.0, 0.0));
    const auto heavyResult = estimate(heavy, domain::makeWindImpact(3.0, 0.0, 0.0));

    ASSERT_TRUE(lightResult.duration.has_value());
    ASSERT_TRUE(heavyResult.duration.has_value());
    EXPECT_GT(lightResult.values.estimatedDurationMinutes, heavyResult.values.estimatedDurationMinutes);
}

TEST(TestDurationEstimatorTest, Estimator_WhenAngleOfAttackIncreases_DurationDecreases) {
    const auto straight = estimate(validUav(), domain::makeWindImpact(3.0, 0.0, 0.0));
    const auto angled = estimate(validUav(), domain::makeWindImpact(3.0, 0.0, 35.0));

    ASSERT_TRUE(straight.duration.has_value());
    ASSERT_TRUE(angled.duration.has_value());
    EXPECT_GT(straight.values.estimatedDurationMinutes, angled.values.estimatedDurationMinutes);
}

TEST(TestDurationEstimatorTest, Estimator_WhenRequiredThrustExceedsAvailable_ReturnsDiagnosticError) {
    auto uav = validUav();
    uav.motor.maxThrustKg = 0.5;

    const auto result = estimate(uav, domain::makeWindImpact(7.0, 0.0, 60.0));

    EXPECT_TRUE(hasDiagnostic(result.errors, domain::TestDurationDiagnosticCode::RequiredThrustExceedsAvailable));
    EXPECT_FALSE(result.duration.has_value());
}

TEST(TestDurationEstimatorTest, Estimator_WhenCurrentExceedsMotorPeak_ReturnsDiagnosticWarning) {
    auto uav = validUav();
    uav.motor.peakCurrentA = 10.0;

    const auto result = estimate(uav, domain::makeWindImpact(7.0, 0.0, 70.0));

    EXPECT_TRUE(hasDiagnostic(result.warnings, domain::TestDurationDiagnosticCode::MotorPeakCurrentExceeded));
}

TEST(TestDurationEstimatorTest, Estimator_WhenCurrentExceedsBatteryDischarge_ReturnsDiagnosticWarning) {
    auto uav = validUav();
    uav.battery.dischargeRateC = 1.0;

    const auto result = estimate(uav, domain::makeWindImpact(5.0, 0.0, 45.0));

    EXPECT_TRUE(hasDiagnostic(result.warnings, domain::TestDurationDiagnosticCode::BatteryDischargeCurrentExceeded));
}

TEST(TestDurationEstimatorTest, Estimator_WhenBatteryCapacityMissing_ReturnsError) {
    auto uav = validUav();
    uav.battery.capacityMah = 0;

    const auto result = estimate(uav, domain::makeWindImpact(3.0, 0.0, 0.0));

    EXPECT_TRUE(hasDiagnostic(result.errors, domain::TestDurationDiagnosticCode::BatteryCapacityMissing));
    EXPECT_FALSE(result.duration.has_value());
}

TEST(TestDurationEstimatorTest, Estimator_WhenFrontalAreaMissing_UsesFallbackAndWarning) {
    auto uav = validUav();
    uav.frame.frontalAreaM2 = 0.0;

    const auto result = estimate(uav, domain::makeWindImpact(3.0, 0.0, 0.0));

    EXPECT_TRUE(hasDiagnostic(result.warnings, domain::TestDurationDiagnosticCode::FrontalAreaFallbackUsed));
    EXPECT_DOUBLE_EQ(result.values.frontalAreaM2, domain::TestDurationEstimatorCalibration{}.fallbackFrontalAreaM2);
    EXPECT_TRUE(result.duration.has_value());
}

TEST(TestDurationEstimatorTest, Estimator_WhenDragCoefficientMissing_UsesFallbackAndWarning) {
    auto uav = validUav();
    uav.frame.dragCoefficient = 0.0;

    const auto result = estimate(uav, domain::makeWindImpact(3.0, 0.0, 0.0));

    EXPECT_TRUE(hasDiagnostic(result.warnings, domain::TestDurationDiagnosticCode::DragCoefficientFallbackUsed));
    EXPECT_DOUBLE_EQ(result.values.dragCoefficient, domain::TestDurationEstimatorCalibration{}.fallbackDragCoefficient);
    EXPECT_TRUE(result.duration.has_value());
}

TEST(TestDurationEstimatorTest, Estimator_WhenTotalWeightMissing_EstimatesMassAndWarns) {
    auto uav = validUav();
    uav.frame.totalWeightKg = 0.0;
    uav.frame.flightPayload = 1.5;
    uav.battery.weight = 2.4;
    uav.motor.weight = 0.5;

    const auto result = estimate(uav, domain::makeWindImpact(3.0, 0.0, 0.0));

    EXPECT_TRUE(hasDiagnostic(result.warnings, domain::TestDurationDiagnosticCode::TotalMassEstimated));
    EXPECT_DOUBLE_EQ(result.values.totalMassKg, 1.5 + 2.4 + 0.5 * 4.0);
    EXPECT_TRUE(result.duration.has_value());
}

} // namespace
