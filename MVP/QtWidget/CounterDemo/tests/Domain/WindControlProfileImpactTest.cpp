#include "../../src/Domain/WindControlProfileImpact.hpp"

#include <gtest/gtest.h>

namespace {

TEST(WindControlProfileImpactTest, ResolvesWindImpactByElapsedSeconds) {
    domain::WindControlProfile profile{};
    profile.sampleIntervalSeconds = 1.0;
    profile.samples = {
        domain::WindControlSample{.timeSeconds = 0.0, .timeMinutes = 0.0, .beaufort = domain::Beaufort::from(1.0)},
        domain::WindControlSample{
            .timeSeconds = 1.0, .timeMinutes = 1.0 / 60.0, .beaufort = domain::Beaufort::from(2.0)},
        domain::WindControlSample{
            .timeSeconds = 2.0, .timeMinutes = 2.0 / 60.0, .beaufort = domain::Beaufort::from(3.0)},
    };
    const auto base = domain::makeWindImpact(0.0, 90.0, 4.0, domain::Expression{.value = "x"});

    const auto impact = domain::windImpactAt(profile, domain::ElapsedSeconds::from(1), base);

    ASSERT_TRUE(impact.has_value());
    EXPECT_DOUBLE_EQ(impact->beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(impact->direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(impact->angleOfAttack.degrees(), 4.0);
    EXPECT_EQ(impact->formula.value, "x");
}

TEST(WindControlProfileImpactTest, ClampsElapsedTimeToLastProfileSample) {
    domain::WindControlProfile profile{};
    profile.sampleIntervalSeconds = 1.0;
    profile.samples = {
        domain::WindControlSample{.timeSeconds = 0.0, .timeMinutes = 0.0, .beaufort = domain::Beaufort::from(1.0)},
        domain::WindControlSample{
            .timeSeconds = 1.0, .timeMinutes = 1.0 / 60.0, .beaufort = domain::Beaufort::from(2.0)},
    };

    const auto impact = domain::windImpactAt(profile, domain::ElapsedSeconds::from(100), domain::WindImpact{});

    ASSERT_TRUE(impact.has_value());
    EXPECT_DOUBLE_EQ(impact->beaufort.value(), 2.0);
}

TEST(WindControlProfileImpactTest, EmptyProfileHasNoImpact) {
    const auto impact =
        domain::windImpactAt(domain::WindControlProfile{}, domain::ElapsedSeconds::from(0), domain::WindImpact{});

    EXPECT_FALSE(impact.has_value());
}

} // namespace
