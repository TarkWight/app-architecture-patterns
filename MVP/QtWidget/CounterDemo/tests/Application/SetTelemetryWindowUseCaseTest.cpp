#include "../../src/Application/UseCases/SetTelemetryWindowUseCase.hpp"

#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/AxisTelemetrySample.hpp"

#include <gtest/gtest.h>

namespace {

domain::AxisTelemetrySample validSampleAt(double timestampSeconds) {
    domain::AxisTelemetrySample sample{};
    sample.timestampSeconds = timestampSeconds;
    sample.valid = true;
    return sample;
}

TEST(SetTelemetryWindowUseCaseTest, ManualWindowSelectionDisablesTailFollowing) {
    application::session::SessionState state{};
    application::useCases::SetTelemetryWindowUseCase useCase{state};

    useCase.execute(domain::TelemetryWindowEnd::fromSeconds(42.5));

    EXPECT_FALSE(state.get().telemetry.telemetryFollowTail);
    EXPECT_DOUBLE_EQ(state.get().telemetry.telemetryWindowEndSeconds.seconds(), 42.5);
}

TEST(SetTelemetryWindowUseCaseTest, FollowTailMovesWindowToTelemetryHistoryEnd) {
    application::session::SessionState state{};
    application::useCases::SetTelemetryWindowUseCase useCase{state};
    state.appendTelemetrySample(validSampleAt(10.0));
    state.appendTelemetrySample(validSampleAt(42.5));
    useCase.execute(domain::TelemetryWindowEnd::fromSeconds(1.0));

    useCase.followTail();

    EXPECT_TRUE(state.get().telemetry.telemetryFollowTail);
    EXPECT_DOUBLE_EQ(state.get().telemetry.telemetryWindowEndSeconds.seconds(), 32.5);
}

} // namespace
