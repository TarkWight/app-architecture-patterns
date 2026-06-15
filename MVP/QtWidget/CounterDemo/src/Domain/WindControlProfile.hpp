#ifndef WINDCONTROLPROFILE_HPP
#define WINDCONTROLPROFILE_HPP

#include "WindImpact.hpp"

#include <algorithm>
#include <vector>

namespace domain {

constexpr double windControlProfileSampleIntervalSeconds = 1.0;

class WindControlSampleTime final {
  public:
    static WindControlSampleTime fromSeconds(double rawSeconds) {
        return WindControlSampleTime{std::max(0.0, rawSeconds)};
    }

    [[nodiscard]] double seconds() const {
        return rawSeconds;
    }

    [[nodiscard]] double minutes() const {
        return rawSeconds / 60.0;
    }

  private:
    explicit WindControlSampleTime(double seconds) : rawSeconds(seconds) {
    }

    double rawSeconds{0.0};
};

struct WindControlSample {
    WindControlSampleTime time{WindControlSampleTime::fromSeconds(0.0)};
    Beaufort beaufort{Beaufort::from(0.0)};
};

struct WindControlProfile {
    int durationMinutes{0};
    double sampleIntervalSeconds{windControlProfileSampleIntervalSeconds};
    std::vector<WindControlSample> samples{};
};

} // namespace domain

#endif // WINDCONTROLPROFILE_HPP
