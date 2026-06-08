#ifndef WINDCONTROLPROFILE_HPP
#define WINDCONTROLPROFILE_HPP

#include "WindProfile.hpp"

#include <vector>

namespace domain {

constexpr int temporaryFormulaProfileDurationMinutes = 48;
constexpr double windControlProfileSampleIntervalSeconds = 1.0;

struct WindControlSample {
    double timeSeconds{0.0};
    double timeMinutes{0.0};
    Beaufort beaufort{Beaufort::from(0.0)};
};

struct WindControlProfile {
    int durationMinutes{temporaryFormulaProfileDurationMinutes};
    double sampleIntervalSeconds{windControlProfileSampleIntervalSeconds};
    std::vector<WindControlSample> samples{};
};

} // namespace domain

#endif // WINDCONTROLPROFILE_HPP
