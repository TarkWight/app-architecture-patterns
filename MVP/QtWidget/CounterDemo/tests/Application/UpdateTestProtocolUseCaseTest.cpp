#include "../../src/Application/UseCases/UpdateTestProtocolUseCase.hpp"

#include "../../src/Application/Session/SessionState.hpp"

#include <gtest/gtest.h>

namespace {

TEST(UpdateTestProtocolUseCaseTest, ManualModeSelectsManualStandControlAndFreeRunTime) {
    application::session::SessionState state{};
    application::useCases::UpdateTestProtocolUseCase useCase{state};

    useCase.updateMode("manual");

    EXPECT_EQ(state.get().protocol.testProtocol.testMode, domain::TestMode::Manual);
    EXPECT_EQ(state.get().control.standControlMode, domain::StandControlMode::Manual);
    EXPECT_EQ(state.get().protocol.testTimeSource, domain::TestTimeSource::FreeRun);
    EXPECT_EQ(state.get().execution.testTimeDirection, domain::TestTimeDirection::CountUp);
}

TEST(UpdateTestProtocolUseCaseTest, HybridModeSelectsHybridStandControlAndCalculatedTime) {
    application::session::SessionState state{};
    application::useCases::UpdateTestProtocolUseCase useCase{state};

    useCase.updateMode("hybrid");

    EXPECT_EQ(state.get().protocol.testProtocol.testMode, domain::TestMode::Hybrid);
    EXPECT_EQ(state.get().control.standControlMode, domain::StandControlMode::Hybrid);
    EXPECT_EQ(state.get().protocol.testTimeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(state.get().execution.testTimeDirection, domain::TestTimeDirection::CountDown);
}

TEST(UpdateTestProtocolUseCaseTest, AutomaticModeSelectsPresetStandControlAndCalculatedTime) {
    application::session::SessionState state{};
    application::useCases::UpdateTestProtocolUseCase useCase{state};

    useCase.updateMode("automatic");

    EXPECT_EQ(state.get().protocol.testProtocol.testMode, domain::TestMode::Automatic);
    EXPECT_EQ(state.get().control.standControlMode, domain::StandControlMode::PresetScenario);
    EXPECT_EQ(state.get().protocol.testTimeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(state.get().execution.testTimeDirection, domain::TestTimeDirection::CountDown);
}

TEST(UpdateTestProtocolUseCaseTest, PublishesOneConsistentStateSnapshotForModeChange) {
    application::session::SessionState state{};
    application::useCases::UpdateTestProtocolUseCase useCase{state};
    int notifications = 0;

    [[maybe_unused]] auto subscription =
        state.subscribe([&notifications](const application::session::SessionStateData & /*data*/) { ++notifications; });
    notifications = 0;

    useCase.updateMode("automatic");

    EXPECT_EQ(notifications, 1);
    EXPECT_EQ(state.get().protocol.testProtocol.testMode, domain::TestMode::Automatic);
    EXPECT_EQ(state.get().control.standControlMode, domain::StandControlMode::PresetScenario);
    EXPECT_EQ(state.get().protocol.testTimeSource, domain::TestTimeSource::AutoCalculated);
    EXPECT_EQ(state.get().execution.testTimeDirection, domain::TestTimeDirection::CountDown);
}

TEST(UpdateTestProtocolUseCaseTest, ProgramChangeUpdatesFormulaAndPublishesOneConsistentSnapshot) {
    application::session::SessionState state{};
    application::useCases::UpdateTestProtocolUseCase useCase{state};
    int notifications = 0;
    domain::TestProgram observedProgram{domain::TestProgram::Custom};
    std::string observedExpression{};

    [[maybe_unused]] auto subscription = state.subscribe([&](const application::session::SessionStateData &data) {
        ++notifications;
        observedProgram = data.protocol.testProtocol.testProgram;
        observedExpression = data.control.functionExpression.value;
    });
    notifications = 0;

    useCase.updateProgram("test2");

    EXPECT_EQ(notifications, 1);
    EXPECT_EQ(observedProgram, domain::TestProgram::MaximumWindLoad);
    EXPECT_EQ(observedExpression, "60 * sin(0.0053 * x)");
    EXPECT_EQ(state.get().protocol.testProtocol.testProgram, domain::TestProgram::MaximumWindLoad);
    EXPECT_EQ(state.get().control.functionExpression.value, "60 * sin(0.0053 * x)");
}

TEST(UpdateTestProtocolUseCaseTest, CustomProgramKeepsCurrentFormula) {
    application::session::SessionState state{};
    state.setFunctionExpression("operator_formula");
    application::useCases::UpdateTestProtocolUseCase useCase{state};

    useCase.updateProgram("custom");

    EXPECT_EQ(state.get().protocol.testProtocol.testProgram, domain::TestProgram::Custom);
    EXPECT_EQ(state.get().control.functionExpression.value, "operator_formula");
}

} // namespace
