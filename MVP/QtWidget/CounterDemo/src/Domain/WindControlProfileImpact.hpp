#ifndef WINDCONTROLPROFILEIMPACT_HPP
#define WINDCONTROLPROFILEIMPACT_HPP

#include "Time.hpp"
#include "WindControlProfile.hpp"
#include "WindProfile.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

namespace domain {

inline std::optional<WindProfile> windImpactAt(const WindControlProfile &profile, ElapsedSeconds elapsed,
                                               const WindProfile &baseImpact) {
    if (profile.samples.empty() || profile.sampleIntervalSeconds <= 0.0) {
        return std::nullopt;
    }

    const auto requestedIndex =
        static_cast<std::size_t>(std::floor(static_cast<double>(elapsed.value()) / profile.sampleIntervalSeconds));
    const auto sampleIndex = std::min(requestedIndex, profile.samples.size() - 1);
    const auto &sample = profile.samples.at(sampleIndex);

    return WindProfile{.beaufort = sample.beaufort,
                       .direction = baseImpact.direction,
                       .angleOfAttack = baseImpact.angleOfAttack,
                       .formula = baseImpact.formula};
}

} // namespace domain

#endif // WINDCONTROLPROFILEIMPACT_HPP
