#ifndef CONTROLPROFILETIMING_HPP
#define CONTROLPROFILETIMING_HPP

#include "TestProtocol.hpp"
#include "TestTimeSource.hpp"
#include "Time.hpp"

namespace domain {

struct ControlProfileTiming {
    bool formulaEnabled{false};
    DurationMinutes duration{DurationMinutes::optional(0)};
};

inline ControlProfileTiming determineControlProfileTiming(TestMode mode, TestTimeSource source,
                                                          DurationMinutes estimatedDuration,
                                                          DurationMinutes operatorDuration) {
    switch (mode) {
    case TestMode::Manual:
        return ControlProfileTiming{.formulaEnabled = false, .duration = DurationMinutes::optional(0)};

    case TestMode::Automatic:
        return ControlProfileTiming{.formulaEnabled = true, .duration = estimatedDuration};

    case TestMode::Hybrid:
        if (source == TestTimeSource::OperatorDefined) {
            return ControlProfileTiming{.formulaEnabled = true, .duration = operatorDuration};
        }

        return ControlProfileTiming{.formulaEnabled = true, .duration = estimatedDuration};
    }

    return ControlProfileTiming{.formulaEnabled = false, .duration = DurationMinutes::optional(0)};
}

} // namespace domain

#endif // CONTROLPROFILETIMING_HPP
