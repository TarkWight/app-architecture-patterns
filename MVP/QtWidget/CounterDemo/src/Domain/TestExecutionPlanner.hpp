#pragma once

#include "TestProtocol.hpp"
#include "TestTimeDirection.hpp"
#include "TestTimeSource.hpp"
#include "Time.hpp"

namespace domain {

struct TestExecutionPlan {
    DurationMinutes activeDuration{DurationMinutes::optional(0)};
    TestTimeDirection direction{TestTimeDirection::CountUp};

    [[nodiscard]] RemainingSeconds initialRemaining() const {
        if (direction == TestTimeDirection::CountDown) {
            return RemainingSeconds::from(activeDuration.value() * 60);
        }

        return RemainingSeconds::from(0);
    }

    [[nodiscard]] RemainingSeconds remainingAt(ElapsedSeconds elapsed) const {
        if (direction != TestTimeDirection::CountDown) {
            return RemainingSeconds::from(0);
        }

        return RemainingSeconds::from((activeDuration.value() * 60) - elapsed.value());
    }

    [[nodiscard]] bool isCompletedAt(ElapsedSeconds elapsed) const {
        return direction == TestTimeDirection::CountDown && remainingAt(elapsed).value() == 0;
    }
};

struct TestExecutionStopPlan {
    ElapsedSeconds elapsed{ElapsedSeconds::from(0)};
    RemainingSeconds remaining{RemainingSeconds::from(0)};
};

class TestExecutionPlanner final {
  public:
    [[nodiscard]] static TestExecutionPlan plan(const TestProtocol &protocol, TestTimeSource timeSource,
                                                DurationMinutes estimatedDuration, DurationMinutes operatorDuration) {
        if (protocol.testMode == TestMode::Manual) {
            return TestExecutionPlan{};
        }

        switch (timeSource) {
        case TestTimeSource::AutoCalculated:
            return TestExecutionPlan{.activeDuration = estimatedDuration, .direction = TestTimeDirection::CountDown};
        case TestTimeSource::OperatorDefined:
            return TestExecutionPlan{.activeDuration = operatorDuration, .direction = TestTimeDirection::CountDown};
        case TestTimeSource::FreeRun:
            return TestExecutionPlan{};
        }

        return TestExecutionPlan{};
    }

    [[nodiscard]] static TestExecutionStopPlan resetAfterStop(DurationMinutes activeDuration,
                                                              TestTimeDirection direction) {
        if (direction == TestTimeDirection::CountDown) {
            return TestExecutionStopPlan{.elapsed = ElapsedSeconds::from(0),
                                         .remaining = RemainingSeconds::from(activeDuration.value() * 60)};
        }

        return TestExecutionStopPlan{};
    }
};

} // namespace domain
