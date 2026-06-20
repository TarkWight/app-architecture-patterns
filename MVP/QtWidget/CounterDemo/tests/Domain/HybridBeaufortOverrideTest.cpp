#include "../../src/Domain/HybridBeaufortOverride.hpp"

#include <gtest/gtest.h>

namespace {

constexpr double defaultTolerance = 0.0001;

domain::HybridBeaufortOverrideCalibration calibration() {
    return domain::HybridBeaufortOverrideCalibration{
        .rampToOverride = domain::DurationSeconds::from(2.0),
        .hold = domain::DurationSeconds::from(1.0),
        .returnToScenario = domain::DurationSeconds::from(5.0),
    };
}

domain::HybridBeaufortOverride defaultOverride(double scenarioBeaufort = 2.0, double operatorBeaufort = 6.0,
                                               int startedAt = 10) {
    return domain::HybridBeaufortOverridePolicy::startOverride(domain::Beaufort::from(scenarioBeaufort),
                                                               domain::Beaufort::from(operatorBeaufort),
                                                               domain::ElapsedSeconds::from(startedAt), calibration());
}

} // namespace

TEST(HybridBeaufortOverrideTest, StartOverrideStoresScenarioOperatorValuesAndStartTime) {
    const auto overrideState = domain::HybridBeaufortOverridePolicy::startOverride(
        domain::Beaufort::from(3.0), domain::Beaufort::from(5.0), domain::ElapsedSeconds::from(42), calibration());

    EXPECT_DOUBLE_EQ(overrideState.startScenarioBeaufort.value(), 3.0);
    EXPECT_DOUBLE_EQ(overrideState.operatorBeaufort.value(), 5.0);
    EXPECT_EQ(overrideState.startedAt.value(), 42);
    EXPECT_DOUBLE_EQ(overrideState.rampToOverride.value(), 2.0);
    EXPECT_DOUBLE_EQ(overrideState.hold.value(), 1.0);
    EXPECT_DOUBLE_EQ(overrideState.returnToScenario.value(), 5.0);
}

TEST(HybridBeaufortOverrideTest, RampMovesFromScenarioBeaufortToOperatorBeaufort) {
    const auto overrideState = defaultOverride(2.0, 6.0, 10);

    const auto value = domain::HybridBeaufortOverridePolicy::resolveBeaufort(overrideState, domain::Beaufort::from(2.0),
                                                                             domain::ElapsedSeconds::from(11));

    EXPECT_NEAR(value.value(), 4.0, defaultTolerance);
}

TEST(HybridBeaufortOverrideTest, HoldKeepsOperatorBeaufort) {
    const auto overrideState = defaultOverride(2.0, 6.0, 10);

    const auto value = domain::HybridBeaufortOverridePolicy::resolveBeaufort(overrideState, domain::Beaufort::from(3.0),
                                                                             domain::ElapsedSeconds::from(12));

    EXPECT_DOUBLE_EQ(value.value(), 6.0);
}

TEST(HybridBeaufortOverrideTest, ReturnMovesFromOperatorBeaufortToCurrentScenarioBeaufort) {
    const auto overrideState = defaultOverride(2.0, 6.0, 10);

    const auto value = domain::HybridBeaufortOverridePolicy::resolveBeaufort(overrideState, domain::Beaufort::from(1.0),
                                                                             domain::ElapsedSeconds::from(16));

    EXPECT_NEAR(value.value(), 3.0, defaultTolerance);
}

TEST(HybridBeaufortOverrideTest, CompletedOverrideReturnsScenarioBeaufort) {
    const auto overrideState = defaultOverride(2.0, 6.0, 10);

    const auto value = domain::HybridBeaufortOverridePolicy::resolveBeaufort(overrideState, domain::Beaufort::from(4.0),
                                                                             domain::ElapsedSeconds::from(18));

    EXPECT_DOUBLE_EQ(value.value(), 4.0);
    EXPECT_TRUE(domain::HybridBeaufortOverridePolicy::isCompleted(overrideState, domain::ElapsedSeconds::from(18)));
}

TEST(HybridImpactResolverTest, WithoutOverrideUsesScenarioBeaufort) {
    const auto impact = domain::HybridImpactResolver::resolve(domain::HybridImpactResolutionInput{
        .scenarioBeaufort = domain::Beaufort::from(3.0),
        .operatorDirection = domain::WindDirection::from(90.0),
        .operatorAngleOfAttack = domain::AngleOfAttack::from(15.0),
        .overrideState = std::nullopt,
        .elapsed = domain::ElapsedSeconds::from(0),
    });

    EXPECT_DOUBLE_EQ(impact.beaufort.value(), 3.0);
}

TEST(HybridImpactResolverTest, WithOverrideUsesOverriddenBeaufort) {
    const auto overrideState = defaultOverride(2.0, 6.0, 10);

    const auto impact = domain::HybridImpactResolver::resolve(domain::HybridImpactResolutionInput{
        .scenarioBeaufort = domain::Beaufort::from(2.0),
        .operatorDirection = domain::WindDirection::from(90.0),
        .operatorAngleOfAttack = domain::AngleOfAttack::from(15.0),
        .overrideState = overrideState,
        .elapsed = domain::ElapsedSeconds::from(11),
    });

    EXPECT_NEAR(impact.beaufort.value(), 4.0, defaultTolerance);
}

TEST(HybridImpactResolverTest, AlwaysUsesOperatorDirection) {
    const auto overrideState = defaultOverride();

    const auto impact = domain::HybridImpactResolver::resolve(domain::HybridImpactResolutionInput{
        .scenarioBeaufort = domain::Beaufort::from(2.0),
        .operatorDirection = domain::WindDirection::from(270.0),
        .operatorAngleOfAttack = domain::AngleOfAttack::from(15.0),
        .overrideState = overrideState,
        .elapsed = domain::ElapsedSeconds::from(11),
    });

    EXPECT_DOUBLE_EQ(impact.direction.degrees(), 270.0);
}

TEST(HybridImpactResolverTest, AlwaysUsesOperatorAngleOfAttack) {
    const auto overrideState = defaultOverride();

    const auto impact = domain::HybridImpactResolver::resolve(domain::HybridImpactResolutionInput{
        .scenarioBeaufort = domain::Beaufort::from(2.0),
        .operatorDirection = domain::WindDirection::from(270.0),
        .operatorAngleOfAttack = domain::AngleOfAttack::from(-20.0),
        .overrideState = overrideState,
        .elapsed = domain::ElapsedSeconds::from(11),
    });

    EXPECT_DOUBLE_EQ(impact.angleOfAttack.degrees(), -20.0);
}

TEST(HybridBeaufortOverrideTest, WorksForOperatorBeaufortAboveScenario) {
    const auto overrideState = defaultOverride(1.0, 5.0, 0);

    const auto value = domain::HybridBeaufortOverridePolicy::resolveBeaufort(overrideState, domain::Beaufort::from(1.0),
                                                                             domain::ElapsedSeconds::from(1));

    EXPECT_NEAR(value.value(), 3.0, defaultTolerance);
}

TEST(HybridBeaufortOverrideTest, WorksForOperatorBeaufortBelowScenario) {
    const auto overrideState = defaultOverride(6.0, 2.0, 0);

    const auto value = domain::HybridBeaufortOverridePolicy::resolveBeaufort(overrideState, domain::Beaufort::from(6.0),
                                                                             domain::ElapsedSeconds::from(1));

    EXPECT_NEAR(value.value(), 4.0, defaultTolerance);
}
