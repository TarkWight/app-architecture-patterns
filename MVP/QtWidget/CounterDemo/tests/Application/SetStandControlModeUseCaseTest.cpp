#include "../../src/Application/UseCases/SetStandControlModeUseCase.hpp"

#include "../../src/Application/Session/SessionState.hpp"

#include <gtest/gtest.h>

namespace {

TEST(SetStandControlModeUseCaseTest, SynchronizesManualStandModeWithManualTestMode) {
    application::session::SessionState state{};
    application::useCases::SetStandControlModeUseCase useCase{state};

    useCase.execute(domain::StandControlMode::Manual);

    EXPECT_EQ(state.get().standControlMode, domain::StandControlMode::Manual);
    EXPECT_EQ(state.get().testProtocol.testMode, domain::TestMode::Manual);
}

TEST(SetStandControlModeUseCaseTest, SynchronizesHybridStandModeWithHybridTestMode) {
    application::session::SessionState state{};
    application::useCases::SetStandControlModeUseCase useCase{state};

    useCase.execute(domain::StandControlMode::Hybrid);

    EXPECT_EQ(state.get().standControlMode, domain::StandControlMode::Hybrid);
    EXPECT_EQ(state.get().testProtocol.testMode, domain::TestMode::Hybrid);
}

TEST(SetStandControlModeUseCaseTest, SynchronizesPresetScenarioStandModeWithAutomaticTestMode) {
    application::session::SessionState state{};
    application::useCases::SetStandControlModeUseCase useCase{state};

    useCase.execute(domain::StandControlMode::PresetScenario);

    EXPECT_EQ(state.get().standControlMode, domain::StandControlMode::PresetScenario);
    EXPECT_EQ(state.get().testProtocol.testMode, domain::TestMode::Automatic);
}

} // namespace
