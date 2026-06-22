#include "../../src/Domain/WindControlProfileCalculator.hpp"

#include <gtest/gtest.h>

namespace {

TEST(WindControlProfileCalculatorTest, BuildsOneSecondSamplesForDuration) {
    const auto profile =
        domain::buildWindControlProfile(domain::DurationMinutes::required(2), [](double minute) { return minute; });

    EXPECT_EQ(profile.duration.value(), 2);
    EXPECT_DOUBLE_EQ(profile.sampleIntervalSeconds, domain::windControlProfileSampleIntervalSeconds);
    EXPECT_EQ(profile.samples.size(), static_cast<std::size_t>(2 * 60));

    EXPECT_DOUBLE_EQ(profile.samples.at(0).time.seconds(), 0.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(0).time.minutes(), 0.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).time.seconds(), 60.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).time.minutes(), 1.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).beaufort.value(), 1.0);
}

TEST(WindControlSampleTimeTest, ClampsNegativeSecondsToZero) {
    const auto time = domain::WindControlSampleTime::fromSeconds(-1.0);

    EXPECT_DOUBLE_EQ(time.seconds(), 0.0);
    EXPECT_DOUBLE_EQ(time.minutes(), 0.0);
}

TEST(WindControlProfileCalculatorTest, ClampsFunctionOutputToOperationalBeaufortRange) {
    const auto profile = domain::buildWindControlProfile(domain::DurationMinutes::required(2),
                                                         [](double minute) { return minute < 1.0 ? -1.0 : 9.0; });

    EXPECT_DOUBLE_EQ(profile.samples.at(0).beaufort.value(), domain::minOperationalBeaufort);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).beaufort.value(), domain::maxOperationalBeaufort);
}

} // namespace
