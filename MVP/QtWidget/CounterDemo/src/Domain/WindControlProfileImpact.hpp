#ifndef WINDCONTROLPROFILEIMPACT_HPP
#define WINDCONTROLPROFILEIMPACT_HPP

#include "Time.hpp"
#include "WindControlProfile.hpp"
#include "WindImpact.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

namespace domain {

inline std::optional<WindImpact> windImpactAt(const WindControlProfile &profile, ElapsedSeconds elapsed,
                                              const WindImpact &baseImpact) {
    if (profile.samples.empty() || profile.sampleIntervalSeconds <= 0.0) {
        return std::nullopt;
    }

    const auto requestedIndex =
        static_cast<std::size_t>(std::floor(static_cast<double>(elapsed.value()) / profile.sampleIntervalSeconds));
    const auto sampleIndex = std::min(requestedIndex, profile.samples.size() - 1);
    const auto &sample = profile.samples.at(sampleIndex);

    return WindImpact{.beaufort = sample.beaufort,
                      .direction = baseImpact.direction,
                      .angleOfAttack = baseImpact.angleOfAttack,
                      .formula = baseImpact.formula};
}

} // namespace domain

#endif // WINDCONTROLPROFILEIMPACT_HPP
