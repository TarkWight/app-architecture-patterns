#include "../../src/Application/Services/AppliedStandImpactSender.hpp"

#include "../../src/Application/Ports/ITelemetryClient.hpp"
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

TEST(AppliedStandImpactSenderTest, SendsMappedAxisCommandsWithYawOscillation) {
    TelemetryClientSpy telemetryClient{};
    application::services::AppliedStandImpactSender sender{telemetryClient};
    domain::TestProtocol protocol{};
    protocol.droneParameters = {
        {"uav_model", "Модель БПЛА", "BAS-1"},
        {"battery_weight", "Вес аккумулятора", "2,4"},
    };

    sender.send(domain::makeWindImpact(4.0, 70.0, 30.0), domain::ElapsedSeconds::from(3), protocol);

    ASSERT_TRUE(telemetryClient.axis0Command.has_value());
    ASSERT_TRUE(telemetryClient.axis1Command.has_value());
    EXPECT_FLOAT_EQ(telemetryClient.axis0Command->position, 0.0F);
    EXPECT_FLOAT_EQ(telemetryClient.axis0Command->velocity, domain::axisCommandVelocity);
    EXPECT_FLOAT_EQ(telemetryClient.axis0Command->torque, 26.0F);
    EXPECT_FLOAT_EQ(telemetryClient.axis1Command->position, 106.0F);
    EXPECT_FLOAT_EQ(telemetryClient.axis1Command->velocity, domain::axisCommandVelocity);
    EXPECT_FLOAT_EQ(telemetryClient.axis1Command->torque, 20.0F);
}

} // namespace
