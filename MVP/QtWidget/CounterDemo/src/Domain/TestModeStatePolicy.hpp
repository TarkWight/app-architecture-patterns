#pragma once

#include "StandControlMode.hpp"
#include "TestTimeDirection.hpp"
#include "TestTimeSource.hpp"

namespace domain {

struct TestModeState {
    TestMode testMode{TestMode::Manual};
    StandControlMode standControlMode{StandControlMode::Manual};
    TestTimeSource timeSource{TestTimeSource::FreeRun};
    TestTimeDirection timeDirection{TestTimeDirection::CountUp};
};

class TestModeStatePolicy final {
  public:
    [[nodiscard]] static TestModeState fromStandControlMode(StandControlMode mode) {
        return fromTestMode(testModeForStandControlMode(mode));
    }

    [[nodiscard]] static TestModeState fromTestMode(TestMode mode) {
        const auto standMode = standControlModeForTestMode(mode);

        switch (mode) {
        case TestMode::Manual:
            return TestModeState{.testMode = mode,
                                 .standControlMode = standMode,
                                 .timeSource = TestTimeSource::FreeRun,
                                 .timeDirection = TestTimeDirection::CountUp};
        case TestMode::Hybrid:
        case TestMode::Automatic:
            return TestModeState{.testMode = mode,
                                 .standControlMode = standMode,
                                 .timeSource = TestTimeSource::AutoCalculated,
                                 .timeDirection = TestTimeDirection::CountDown};
        }

        return TestModeState{};
    }

    [[nodiscard]] static TestTimeDirection directionForTimeSource(TestTimeSource source) {
        switch (source) {
        case TestTimeSource::AutoCalculated:
        case TestTimeSource::OperatorDefined:
            return TestTimeDirection::CountDown;
        case TestTimeSource::FreeRun:
            return TestTimeDirection::CountUp;
        }

        return TestTimeDirection::CountUp;
    }

    [[nodiscard]] static bool allowsOperatorDuration(TestMode mode) {
        return mode == TestMode::Hybrid;
    }

    [[nodiscard]] static bool usesControlProfile(TestMode mode) {
        return mode != TestMode::Manual;
    }

    [[nodiscard]] static TestTimeSource timeSourceAfterOperatorDuration(TestMode mode, TestTimeSource currentSource) {
        if (allowsOperatorDuration(mode)) {
            return TestTimeSource::OperatorDefined;
        }

        return currentSource;
    }
};

} // namespace domain
