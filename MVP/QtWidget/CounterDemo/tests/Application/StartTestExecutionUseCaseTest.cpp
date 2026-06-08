#include "../../src/Application/UseCases/StartTestExecutionUseCase.hpp"

#include "../../src/Application/Ports/ITestExecutionScheduler.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/TestTimeDirection.hpp"
#include "../../src/Domain/TestTimeSource.hpp"

#include <gtest/gtest.h>

namespace {

class TestExecutionSchedulerSpy final : public application::ports::ITestExecutionScheduler {
  public:
    void start(int initialElapsedSeconds, TickCallback onTick) override {
        started = true;
        initialElapsed = initialElapsedSeconds;
        tick = std::move(onTick);
    }

    void pause() override {
    }

    void resume() override {
    }

    void stop() override {
        stopped = true;
    }

    bool isRunning() const override {
        return started && !stopped;
    }

    bool isPaused() const override {
        return false;
    }

    bool started{false};
    bool stopped{false};
    int initialElapsed{-1};
    TickCallback tick{};
};

TEST(StartTestExecutionUseCaseTest, ManualModeStartsAsStopwatchEvenWhenOperatorDurationIsSelected) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Manual);
    state.setTestTimeSource(domain::TestTimeSource::OperatorDefined);
    state.setOperatorTestDurationMinutes(20);

    TestExecutionSchedulerSpy scheduler{};
    application::useCases::StartTestExecutionUseCase useCase{state, scheduler};

    useCase.execute();

    EXPECT_TRUE(scheduler.started);
    EXPECT_EQ(scheduler.initialElapsed, 0);
    EXPECT_EQ(state.get().testTimeDirection, domain::TestTimeDirection::CountUp);
    EXPECT_EQ(state.get().activeTestDuration.value(), 0);
    EXPECT_EQ(state.get().remaining.value(), 0);
}

} // namespace
