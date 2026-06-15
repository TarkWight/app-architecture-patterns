#ifndef TESTEXECUTIONTRANSITIONS_HPP
#define TESTEXECUTIONTRANSITIONS_HPP

#include "TestExecutionStatus.hpp"

#include <optional>

namespace domain {

constexpr bool canStart(TestExecutionStatus status) {
    switch (status) {
    case TestExecutionStatus::Idle:
    case TestExecutionStatus::Ready:
    case TestExecutionStatus::Completed:
    case TestExecutionStatus::Aborted:
    case TestExecutionStatus::Failed:
        return true;
    case TestExecutionStatus::Running:
    case TestExecutionStatus::Paused:
        return false;
    }
    return false;
}

constexpr bool canPause(TestExecutionStatus status) {
    return status == TestExecutionStatus::Running;
}

constexpr bool canResume(TestExecutionStatus status) {
    return status == TestExecutionStatus::Paused;
}

constexpr bool canStop(TestExecutionStatus status) {
    return status == TestExecutionStatus::Running || status == TestExecutionStatus::Paused;
}

inline std::optional<TestExecutionStatus> transitionAfterStartRequested(TestExecutionStatus status) {
    if (!canStart(status)) {
        return std::nullopt;
    }

    return TestExecutionStatus::Running;
}

inline std::optional<TestExecutionStatus> transitionAfterPauseRequested(TestExecutionStatus status) {
    if (!canPause(status)) {
        return std::nullopt;
    }

    return TestExecutionStatus::Paused;
}

inline std::optional<TestExecutionStatus> transitionAfterResumeRequested(TestExecutionStatus status) {
    if (!canResume(status)) {
        return std::nullopt;
    }

    return TestExecutionStatus::Running;
}

inline std::optional<TestExecutionStatus> transitionAfterStopRequested(TestExecutionStatus status) {
    if (!canStop(status)) {
        return std::nullopt;
    }

    return TestExecutionStatus::Ready;
}

inline std::optional<TestExecutionStatus> transitionAfterExecutionCompleted(TestExecutionStatus status) {
    if (status != TestExecutionStatus::Running) {
        return std::nullopt;
    }

    return TestExecutionStatus::Completed;
}

} // namespace domain

#endif // TESTEXECUTIONTRANSITIONS_HPP
