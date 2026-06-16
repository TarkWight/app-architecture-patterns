#include "../../src/Domain/YawOscillationPolicy.hpp"

#include <gtest/gtest.h>

namespace {

TEST(YawOscillationPolicyTest, ReturnsZeroOffsetWhenBeaufortIsZero) {
    const auto context = domain::StandImpactCalculationContext{
        .impact = domain::makeWindImpact(0.0, 70.0, 30.0),
        .elapsed = domain::ElapsedSeconds::from(3),
        .uavSpecification = std::nullopt,
    };

    const auto offset = domain::YawOscillationPolicy::calculate(context);

    EXPECT_DOUBLE_EQ(offset.degrees(), 0.0);
}

TEST(YawOscillationPolicyTest, UsesDeterministicSinusoidalOffsetBasedOnBeaufort) {
    const auto context = domain::StandImpactCalculationContext{
        .impact = domain::makeWindImpact(4.0, 70.0, 30.0),
        .elapsed = domain::ElapsedSeconds::from(3),
        .uavSpecification = std::nullopt,
    };

    const auto offset = domain::YawOscillationPolicy::calculate(context);

    EXPECT_NEAR(offset.degrees(), 6.0, 0.000001);
}

TEST(YawOscillationPolicyTest, KeepsUavSpecificationAsFutureCalculationContext) {
    domain::UavSpecification specification{};
    specification.frame.flightPayload = 15.0;
    specification.frame.dragRatioPercent = 89.0;
    specification.battery.weight = 2.4;

    const auto contextWithoutSpec = domain::StandImpactCalculationContext{
        .impact = domain::makeWindImpact(4.0, 70.0, 30.0),
        .elapsed = domain::ElapsedSeconds::from(3),
        .uavSpecification = std::nullopt,
    };
    const auto contextWithSpec = domain::StandImpactCalculationContext{
        .impact = domain::makeWindImpact(4.0, 70.0, 30.0),
        .elapsed = domain::ElapsedSeconds::from(3),
        .uavSpecification = specification,
    };

    EXPECT_TRUE(contextWithSpec.uavSpecification.has_value());
    EXPECT_DOUBLE_EQ(domain::YawOscillationPolicy::calculate(contextWithSpec).degrees(),
                     domain::YawOscillationPolicy::calculate(contextWithoutSpec).degrees());
}

} // namespace
