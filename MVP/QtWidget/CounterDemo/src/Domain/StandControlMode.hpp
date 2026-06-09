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

constexpr StandControlMode standControlModeForTestMode(TestMode mode) {
    switch (mode) {
    case TestMode::Manual:
        return StandControlMode::Manual;
    case TestMode::Hybrid:
        return StandControlMode::Hybrid;
    case TestMode::Automatic:
        return StandControlMode::PresetScenario;
    }

    return StandControlMode::Manual;
}

} // namespace domain

#endif // STANDCONTROLMODE_HPP
