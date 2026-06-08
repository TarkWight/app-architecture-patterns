#include "../../src/Application/UseCases/SetControlChartsTabMinutesUseCase.hpp"

#include "../../src/Application/Session/SessionState.hpp"

#include <gtest/gtest.h>

namespace {

TEST(SetControlChartsTabMinutesUseCaseTest, AcceptsOperatorDurationOnlyInHybridMode) {
    application::session::SessionState state{};
    application::useCases::SetControlChartsTabMinutesUseCase useCase{state};

    state.setTestProtocolMode(domain::TestMode::Hybrid);
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);

    useCase.execute(37);

    EXPECT_EQ(state.get().controlChartsTabMinutes.value(), 37);
    EXPECT_EQ(state.get().operatorTestDuration.value(), 37);
    EXPECT_EQ(state.get().testTimeSource, domain::TestTimeSource::OperatorDefined);
}

TEST(SetControlChartsTabMinutesUseCaseTest, IgnoresOperatorDurationInManualMode) {
    application::session::SessionState state{};
    application::useCases::SetControlChartsTabMinutesUseCase useCase{state};

    state.setTestProtocolMode(domain::TestMode::Manual);
    state.setControlChartsTabMinutes(20);
    state.setOperatorTestDurationMinutes(20);
    state.setTestTimeSource(domain::TestTimeSource::FreeRun);

    useCase.execute(37);

    EXPECT_EQ(state.get().controlChartsTabMinutes.value(), 20);
    EXPECT_EQ(state.get().operatorTestDuration.value(), 20);
    EXPECT_EQ(state.get().testTimeSource, domain::TestTimeSource::FreeRun);
}

TEST(SetControlChartsTabMinutesUseCaseTest, IgnoresOperatorDurationInAutomaticMode) {
    application::session::SessionState state{};
    application::useCases::SetControlChartsTabMinutesUseCase useCase{state};

    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setControlChartsTabMinutes(20);
    state.setOperatorTestDurationMinutes(20);
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);

    useCase.execute(37);

    EXPECT_EQ(state.get().controlChartsTabMinutes.value(), 20);
    EXPECT_EQ(state.get().operatorTestDuration.value(), 20);
    EXPECT_EQ(state.get().testTimeSource, domain::TestTimeSource::AutoCalculated);
}

} // namespace
