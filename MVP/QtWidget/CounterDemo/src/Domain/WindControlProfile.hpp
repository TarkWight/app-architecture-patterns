#ifndef WINDCONTROLPROFILE_HPP
#define WINDCONTROLPROFILE_HPP

#include "WindImpact.hpp"

#include <vector>

namespace domain {

constexpr double windControlProfileSampleIntervalSeconds = 1.0;

struct WindControlSample {
    double timeSeconds{0.0};
    double timeMinutes{0.0};
    Beaufort beaufort{Beaufort::from(0.0)};
};

struct WindControlProfile {
    int durationMinutes{0};
    double sampleIntervalSeconds{windControlProfileSampleIntervalSeconds};
    std::vector<WindControlSample> samples{};
};

} // namespace domain

#endif // WINDCONTROLPROFILE_HPP
