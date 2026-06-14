#pragma once

#include "ControlTrace.hpp"
#include "Time.hpp"
#include "WindControlProfileImpact.hpp"

#include <optional>

namespace domain {

struct ScenarioExecutionStep {
    WindImpact impact{};
    ControlTraceSample traceSample{};
};

class ScenarioExecutionEngine final {
  public:
    [[nodiscard]] static std::optional<ScenarioExecutionStep>
    advance(const WindControlProfile &profile, ElapsedSeconds elapsed, const WindImpact &baseImpact) {
        const auto impact = windImpactAt(profile, elapsed, baseImpact);
        if (!impact.has_value()) {
            return std::nullopt;
        }

        return ScenarioExecutionStep{
            .impact = *impact,
            .traceSample = ControlTraceSample{.timeSeconds = static_cast<double>(elapsed.value()),
                                              .targetValue = *impact,
                                              .safeCommandValue = *impact},
        };
    }
};

} // namespace domain
