#ifndef TESTEXECUTIONSTATUS_HPP
#define TESTEXECUTIONSTATUS_HPP

namespace domain {

enum class TestExecutionStatus {
    Idle,
    Ready,
    Running,
    Paused,
    Completed,
    Aborted,
    Failed,
};

} // namespace domain

#endif // TESTEXECUTIONSTATUS_HPP
