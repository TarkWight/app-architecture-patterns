#ifndef STANDCONTROLMODE_HPP
#define STANDCONTROLMODE_HPP

#include "TestProtocol.hpp"

namespace domain {

enum class StandControlMode {
    Manual,
    Hybrid,
    PresetScenario,
};

constexpr TestMode testModeForStandControlMode(StandControlMode mode) {
    switch (mode) {
    case StandControlMode::Manual:
        return TestMode::Manual;
    case StandControlMode::Hybrid:
        return TestMode::Hybrid;
    case StandControlMode::PresetScenario:
        return TestMode::Automatic;
    }

    return TestMode::Manual;
}

} // namespace domain

#endif // STANDCONTROLMODE_HPP
