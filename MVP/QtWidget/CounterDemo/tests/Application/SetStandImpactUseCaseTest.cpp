#include "../../src/Application/UseCases/SetStandImpactUseCase.hpp"

#include "../../src/Application/Ports/ITelemetryClient.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/AxisId.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>

namespace {

class TelemetryClientSpy final : public application::ports::ITelemetryClient {
  public:
    void setTelemetryCallback(TelemetryCallback callback) override {
        telemetryCallback = std::move(callback);
    }

    void setStatusCallback(StatusCallback callback) override {
        statusCallback = std::move(callback);
    }

    void setErrorCallback(ErrorCallback callback) override {
        errorCallback = std::move(callback);
    }

    void configureAxis(domain::AxisId /*axisId*/, std::string /*host*/, int /*port*/) override {
    }

    void connectAxis(domain::AxisId /*axisId*/) override {
    }

    void disconnectAxis(domain::AxisId /*axisId*/) override {
    }

    void connectAll() override {
    }

    void disconnectAll() override {
    }

    void startPolling(int /*intervalMs*/) override {
    }

    void stopPolling() override {
    }

    void setAxisCommand(domain::AxisId axisId, domain::AxisControlCommand command) override {
        if (axisId == domain::axis0) {
            axis0Command = command;
        } else if (axisId == domain::axis1) {
            axis1Command = command;
        }
    }

    void pollOnce(domain::AxisId /*axisId*/) override {
    }

    TelemetryCallback telemetryCallback{};
    StatusCallback statusCallback{};
    ErrorCallback errorCallback{};
    std::optional<domain::AxisControlCommand> axis0Command{};
    std::optional<domain::AxisControlCommand> axis1Command{};
};

TEST(SetStandImpactUseCaseTest, RecordsManualTargetAndSafeCommandTraceWhenAppliedImpactIsSent) {
    application::session::SessionState state{};
    state.setElapsedSeconds(7);
    const auto target = domain::makeWindImpact(5.0, 90.0, 4.0);
    const auto safeCommand = domain::makeWindImpact(1.2, 20.0, 2.0);

    TelemetryClientSpy telemetryClient{};
    application::useCases::SetStandImpactUseCase useCase{state, telemetryClient};

    useCase.setTarget(target);
    useCase.setApplied(safeCommand);

    ASSERT_EQ(state.get().controlTrace.size(), 1U);
    EXPECT_DOUBLE_EQ(state.get().controlTrace.front().time.seconds(), 7.0);
    EXPECT_DOUBLE_EQ(state.get().controlTrace.front().targetValue.beaufort.value(), 5.0);
    EXPECT_DOUBLE_EQ(state.get().controlTrace.front().safeCommandValue.beaufort.value(), 1.2);
    ASSERT_TRUE(telemetryClient.axis1Command.has_value());
    EXPECT_FLOAT_EQ(telemetryClient.axis1Command->torque, 6.0F);
}

} // namespace
