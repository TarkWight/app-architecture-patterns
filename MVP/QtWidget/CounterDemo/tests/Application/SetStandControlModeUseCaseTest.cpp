#include "../../src/Application/UseCases/SetStandControlModeUseCase.hpp"

#include "../../src/Application/Session/SessionState.hpp"

#include <gtest/gtest.h>

namespace {

TEST(SetStandControlModeUseCaseTest, SynchronizesManualStandModeWithManualTestMode) {
    application::session::SessionState state{};
    application::useCases::SetStandControlModeUseCase useCase{state};

    useCase.execute(domain::StandControlMode::Manual);

    EXPECT_EQ(state.get().control.standControlMode, domain::StandControlMode::Manual);
    EXPECT_EQ(state.get().protocol.testProtocol.testMode, domain::TestMode::Manual);
    EXPECT_EQ(state.get().protocol.testTimeSource, domain::TestTimeSource::FreeRun);
    EXPECT_EQ(state.get().execution.testTimeDirection, domain::TestTimeDirection::CountUp);
}

TEST(SetStandControlModeUseCaseTest, SynchronizesHybridStandModeWithHybridTestMode) {
    application::session::SessionState state{};
    application::useCases::SetStandControlModeUseCase useCase{state};

    useCase.execute(domain::StandControlMode::Hybrid);

    EXPECT_EQ(state.get().control.standControlMode, domain::StandControlMode::Hybrid);
    EXPECT_EQ(state.get().protocol.testProtocol.testMode, domain::TestMode::Hybrid);
    EXPECT_EQ(state.get().protocol.testTimeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(state.get().execution.testTimeDirection, domain::TestTimeDirection::CountDown);
}

TEST(SetStandControlModeUseCaseTest, SynchronizesPresetScenarioStandModeWithAutomaticTestMode) {
    application::session::SessionState state{};
    application::useCases::SetStandControlModeUseCase useCase{state};

    useCase.execute(domain::StandControlMode::PresetScenario);

    EXPECT_EQ(state.get().control.standControlMode, domain::StandControlMode::PresetScenario);
    EXPECT_EQ(state.get().protocol.testProtocol.testMode, domain::TestMode::Automatic);
    EXPECT_EQ(state.get().protocol.testTimeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(state.get().execution.testTimeDirection, domain::TestTimeDirection::CountDown);
}

TEST(SetStandControlModeUseCaseTest, PublishesOneConsistentStateSnapshot) {
    application::session::SessionState state{};
    application::useCases::SetStandControlModeUseCase useCase{state};
    int notifications = 0;

    [[maybe_unused]] auto subscription =
        state.subscribe([&notifications](const application::session::SessionStateData & /*data*/) { ++notifications; });
    notifications = 0;

    useCase.execute(domain::StandControlMode::Hybrid);

    EXPECT_EQ(notifications, 1);
    EXPECT_EQ(state.get().control.standControlMode, domain::StandControlMode::Hybrid);
    EXPECT_EQ(state.get().protocol.testProtocol.testMode, domain::TestMode::Hybrid);
    EXPECT_EQ(state.get().protocol.testTimeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(state.get().execution.testTimeDirection, domain::TestTimeDirection::CountDown);
}

} // namespace
