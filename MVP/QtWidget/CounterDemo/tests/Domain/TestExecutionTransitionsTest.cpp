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

TEST(TestExecutionTransitionsTest, StartRequestTransitionsOnlyInactiveStatesToRunning) {
    EXPECT_EQ(domain::transitionAfterStartRequested(domain::TestExecutionStatus::Idle),
              domain::TestExecutionStatus::Running);
    EXPECT_EQ(domain::transitionAfterStartRequested(domain::TestExecutionStatus::Ready),
              domain::TestExecutionStatus::Running);
    EXPECT_EQ(domain::transitionAfterStartRequested(domain::TestExecutionStatus::Completed),
              domain::TestExecutionStatus::Running);
    EXPECT_EQ(domain::transitionAfterStartRequested(domain::TestExecutionStatus::Aborted),
              domain::TestExecutionStatus::Running);
    EXPECT_EQ(domain::transitionAfterStartRequested(domain::TestExecutionStatus::Failed),
              domain::TestExecutionStatus::Running);

    EXPECT_FALSE(domain::transitionAfterStartRequested(domain::TestExecutionStatus::Running).has_value());
    EXPECT_FALSE(domain::transitionAfterStartRequested(domain::TestExecutionStatus::Paused).has_value());
}

TEST(TestExecutionTransitionsTest, PauseRequestTransitionsOnlyRunningToPaused) {
    EXPECT_FALSE(domain::transitionAfterPauseRequested(domain::TestExecutionStatus::Idle).has_value());
    EXPECT_FALSE(domain::transitionAfterPauseRequested(domain::TestExecutionStatus::Ready).has_value());
    EXPECT_EQ(domain::transitionAfterPauseRequested(domain::TestExecutionStatus::Running),
              domain::TestExecutionStatus::Paused);
    EXPECT_FALSE(domain::transitionAfterPauseRequested(domain::TestExecutionStatus::Paused).has_value());
    EXPECT_FALSE(domain::transitionAfterPauseRequested(domain::TestExecutionStatus::Completed).has_value());
    EXPECT_FALSE(domain::transitionAfterPauseRequested(domain::TestExecutionStatus::Aborted).has_value());
    EXPECT_FALSE(domain::transitionAfterPauseRequested(domain::TestExecutionStatus::Failed).has_value());
}

TEST(TestExecutionTransitionsTest, ResumeRequestTransitionsOnlyPausedToRunning) {
    EXPECT_FALSE(domain::transitionAfterResumeRequested(domain::TestExecutionStatus::Idle).has_value());
    EXPECT_FALSE(domain::transitionAfterResumeRequested(domain::TestExecutionStatus::Ready).has_value());
    EXPECT_FALSE(domain::transitionAfterResumeRequested(domain::TestExecutionStatus::Running).has_value());
    EXPECT_EQ(domain::transitionAfterResumeRequested(domain::TestExecutionStatus::Paused),
              domain::TestExecutionStatus::Running);
    EXPECT_FALSE(domain::transitionAfterResumeRequested(domain::TestExecutionStatus::Completed).has_value());
    EXPECT_FALSE(domain::transitionAfterResumeRequested(domain::TestExecutionStatus::Aborted).has_value());
    EXPECT_FALSE(domain::transitionAfterResumeRequested(domain::TestExecutionStatus::Failed).has_value());
}

TEST(TestExecutionTransitionsTest, StopRequestTransitionsOnlyActiveStatesToReady) {
    EXPECT_FALSE(domain::transitionAfterStopRequested(domain::TestExecutionStatus::Idle).has_value());
    EXPECT_FALSE(domain::transitionAfterStopRequested(domain::TestExecutionStatus::Ready).has_value());
    EXPECT_EQ(domain::transitionAfterStopRequested(domain::TestExecutionStatus::Running),
              domain::TestExecutionStatus::Ready);
    EXPECT_EQ(domain::transitionAfterStopRequested(domain::TestExecutionStatus::Paused),
              domain::TestExecutionStatus::Ready);
    EXPECT_FALSE(domain::transitionAfterStopRequested(domain::TestExecutionStatus::Completed).has_value());
    EXPECT_FALSE(domain::transitionAfterStopRequested(domain::TestExecutionStatus::Aborted).has_value());
    EXPECT_FALSE(domain::transitionAfterStopRequested(domain::TestExecutionStatus::Failed).has_value());
}

TEST(TestExecutionTransitionsTest, CompletionTransitionsOnlyRunningToCompleted) {
    EXPECT_FALSE(domain::transitionAfterExecutionCompleted(domain::TestExecutionStatus::Idle).has_value());
    EXPECT_FALSE(domain::transitionAfterExecutionCompleted(domain::TestExecutionStatus::Ready).has_value());
    EXPECT_EQ(domain::transitionAfterExecutionCompleted(domain::TestExecutionStatus::Running),
              domain::TestExecutionStatus::Completed);
    EXPECT_FALSE(domain::transitionAfterExecutionCompleted(domain::TestExecutionStatus::Paused).has_value());
    EXPECT_FALSE(domain::transitionAfterExecutionCompleted(domain::TestExecutionStatus::Completed).has_value());
    EXPECT_FALSE(domain::transitionAfterExecutionCompleted(domain::TestExecutionStatus::Aborted).has_value());
    EXPECT_FALSE(domain::transitionAfterExecutionCompleted(domain::TestExecutionStatus::Failed).has_value());
}

} // namespace
