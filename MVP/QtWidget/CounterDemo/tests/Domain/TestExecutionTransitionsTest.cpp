#include "../../src/Domain/TestExecutionTransitions.hpp"

#include <gtest/gtest.h>

namespace {

TEST(TestExecutionTransitionsTest, StartIsAllowedFromNonActiveStates) {
    EXPECT_TRUE(domain::canStart(domain::TestExecutionStatus::Idle));
    EXPECT_TRUE(domain::canStart(domain::TestExecutionStatus::Ready));
    EXPECT_TRUE(domain::canStart(domain::TestExecutionStatus::Completed));
    EXPECT_TRUE(domain::canStart(domain::TestExecutionStatus::Aborted));
    EXPECT_TRUE(domain::canStart(domain::TestExecutionStatus::Failed));
}

TEST(TestExecutionTransitionsTest, StartIsRejectedWhileExecutionIsActive) {
    EXPECT_FALSE(domain::canStart(domain::TestExecutionStatus::Running));
    EXPECT_FALSE(domain::canStart(domain::TestExecutionStatus::Paused));
}

TEST(TestExecutionTransitionsTest, PauseResumeAndStopAreBoundToActiveStates) {
    EXPECT_TRUE(domain::canPause(domain::TestExecutionStatus::Running));
    EXPECT_FALSE(domain::canPause(domain::TestExecutionStatus::Paused));

    EXPECT_TRUE(domain::canResume(domain::TestExecutionStatus::Paused));
    EXPECT_FALSE(domain::canResume(domain::TestExecutionStatus::Running));

    EXPECT_TRUE(domain::canStop(domain::TestExecutionStatus::Running));
    EXPECT_TRUE(domain::canStop(domain::TestExecutionStatus::Paused));
    EXPECT_FALSE(domain::canStop(domain::TestExecutionStatus::Completed));
}

} // namespace
