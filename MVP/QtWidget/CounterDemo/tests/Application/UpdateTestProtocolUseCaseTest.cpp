#include "../../src/Application/UseCases/UpdateTestProtocolUseCase.hpp"

#include "../../src/Application/Session/SessionState.hpp"

#include <gtest/gtest.h>

namespace {

TEST(UpdateTestProtocolUseCaseTest, ManualModeSelectsManualStandControlAndFreeRunTime) {
    application::session::SessionState state{};
    application::useCases::UpdateTestProtocolUseCase useCase{state};

    useCase.updateMode("manual");

    EXPECT_EQ(state.get().testProtocol.testMode, domain::TestMode::Manual);
    EXPECT_EQ(state.get().standControlMode, domain::StandControlMode::Manual);
    EXPECT_EQ(state.get().testTimeSource, domain::TestTimeSource::FreeRun);
    EXPECT_EQ(state.get().testTimeDirection, domain::TestTimeDirection::CountUp);
}

TEST(UpdateTestProtocolUseCaseTest, HybridModeSelectsHybridStandControlAndCalculatedTime) {
    application::session::SessionState state{};
    application::useCases::UpdateTestProtocolUseCase useCase{state};

    useCase.updateMode("hybrid");

    EXPECT_EQ(state.get().testProtocol.testMode, domain::TestMode::Hybrid);
    EXPECT_EQ(state.get().standControlMode, domain::StandControlMode::Hybrid);
    EXPECT_EQ(state.get().testTimeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(state.get().testTimeDirection, domain::TestTimeDirection::CountDown);
}

TEST(UpdateTestProtocolUseCaseTest, AutomaticModeSelectsPresetStandControlAndCalculatedTime) {
    application::session::SessionState state{};
    application::useCases::UpdateTestProtocolUseCase useCase{state};

    useCase.updateMode("automatic");

    EXPECT_EQ(state.get().testProtocol.testMode, domain::TestMode::Automatic);
    EXPECT_EQ(state.get().standControlMode, domain::StandControlMode::PresetScenario);
    EXPECT_EQ(state.get().testTimeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(state.get().testTimeDirection, domain::TestTimeDirection::CountDown);
}

} // namespace
