#ifndef TESTEXECUTIONTRANSITIONS_HPP
#define TESTEXECUTIONTRANSITIONS_HPP

#include "TestExecutionStatus.hpp"

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

} // namespace domain

#endif // TESTEXECUTIONTRANSITIONS_HPP
