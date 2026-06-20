#include "../../src/Domain/SafeWindImpactLimitEstimator.hpp"

#include <gtest/gtest.h>

namespace {

domain::UavSpecification limitTestUav() {
    domain::UavSpecification uav{};
    uav.frame.totalWeightKg = 6.0;
    uav.frame.frontalAreaM2 = 0.25;
    uav.frame.dragCoefficient = 1.0;
    uav.frame.equipmentCurrent = 1.0;
    uav.battery.capacityMah = 18000;
    uav.battery.cellCount = 6;
    uav.battery.cellVoltage = 3.8;
    uav.battery.dischargeRateC = 6.0;
    uav.motor.count = 4;
    uav.motor.maxThrustKg = 2.0;
    uav.motor.peakCurrentA = 30.0;
    uav.motor.hoverCurrentA = 8.0;
    return uav;
}

} // namespace

TEST(SafeWindImpactLimitEstimatorTest, SafeBeaufortDecreasesForWeakerUav) {
    auto weak = limitTestUav();
    auto strong = limitTestUav();
    weak.motor.maxThrustKg = 0.55;
    strong.motor.maxThrustKg = 2.5;

    const auto weakLimits = domain::SafeWindImpactLimitEstimator::estimate(weak, domain::makeWindImpact(0.0, 0.0, 0.0));
    const auto strongLimits =
        domain::SafeWindImpactLimitEstimator::estimate(strong, domain::makeWindImpact(0.0, 0.0, 0.0));

    EXPECT_EQ(weakLimits.status, domain::SafeWindImpactLimitStatus::Available);
    EXPECT_EQ(strongLimits.status, domain::SafeWindImpactLimitStatus::Available);
    EXPECT_LT(weakLimits.maxSafeBeaufort, strongLimits.maxSafeBeaufort);
}

TEST(SafeWindImpactLimitEstimatorTest, SafeBeaufortIncreasesForStrongerUav) {
    auto weak = limitTestUav();
    auto strong = limitTestUav();
    weak.motor.maxThrustKg = 0.55;
    strong.motor.maxThrustKg = 2.5;

    const auto weakLimits = domain::SafeWindImpactLimitEstimator::estimate(weak, domain::makeWindImpact(0.0, 0.0, 0.0));
    const auto strongLimits =
        domain::SafeWindImpactLimitEstimator::estimate(strong, domain::makeWindImpact(0.0, 0.0, 0.0));

    EXPECT_GT(strongLimits.maxSafeBeaufort, weakLimits.maxSafeBeaufort);
}

TEST(SafeWindImpactLimitEstimatorTest, SafeAngleDecreasesWhenBaseBeaufortIsHigh) {
    auto uav = limitTestUav();
    uav.motor.maxThrustKg = 1.9;

    const auto calmLimits = domain::SafeWindImpactLimitEstimator::estimate(uav, domain::makeWindImpact(0.0, 0.0, 0.0));
    const auto windyLimits = domain::SafeWindImpactLimitEstimator::estimate(uav, domain::makeWindImpact(7.0, 0.0, 0.0));

    EXPECT_EQ(calmLimits.status, domain::SafeWindImpactLimitStatus::Available);
    EXPECT_EQ(windyLimits.status, domain::SafeWindImpactLimitStatus::Available);
    EXPECT_LT(windyLimits.maxSafeAbsAngleOfAttack, calmLimits.maxSafeAbsAngleOfAttack);
}

TEST(SafeWindImpactLimitEstimatorTest, MissingCriticalUavDataMakesSafeLimitsUnavailable) {
    auto uav = limitTestUav();
    uav.battery.capacityMah = 0;

    const auto limits = domain::SafeWindImpactLimitEstimator::estimate(uav, domain::makeWindImpact(0.0, 0.0, 0.0));

    EXPECT_EQ(limits.status, domain::SafeWindImpactLimitStatus::Unavailable);
    EXPECT_FALSE(limits.diagnostics.empty());
}
