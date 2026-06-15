#ifndef WINDCONTROLPROFILECALCULATOR_HPP
#define WINDCONTROLPROFILECALCULATOR_HPP

#include "Time.hpp"
#include "WindControlProfile.hpp"

#include <cstddef>
#include <functional>

namespace domain {

inline WindControlProfile buildWindControlProfile(DurationMinutes duration,
                                                  const std::function<double(double)> &beaufortAtMinute) {
    WindControlProfile profile{};
    profile.sampleIntervalSeconds = windControlProfileSampleIntervalSeconds;
    profile.durationMinutes = duration.value();

    const int sampleCount =
        static_cast<int>(static_cast<double>(profile.durationMinutes * 60) / profile.sampleIntervalSeconds);
    profile.samples.reserve(static_cast<std::size_t>(sampleCount));

    for (int index = 0; index < sampleCount; ++index) {
        const double timeSeconds = static_cast<double>(index) * profile.sampleIntervalSeconds;
        const auto time = WindControlSampleTime::fromSeconds(timeSeconds);
        const double rawBeaufort = beaufortAtMinute(time.minutes());

        profile.samples.push_back(WindControlSample{
            .time = time,
            .beaufort = Beaufort::from(rawBeaufort),
        });
    }

    return profile;
}

} // namespace domain

#endif // WINDCONTROLPROFILECALCULATOR_HPP
