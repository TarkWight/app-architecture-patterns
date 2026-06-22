#include "../../src/Domain/TelemetryConnectionPolicy.hpp"

#include <gtest/gtest.h>

namespace {

TEST(TelemetryConnectionPolicyTest, ConnectingStatus_WhenDisconnectingIsNotActive_MarksStandConnecting) {
    const auto decision = domain::TelemetryConnectionPolicy::handleStatus(
        domain::StandConnectionStatus::Configured, domain::TelemetryConnectionStatus::Connecting, false);

    ASSERT_TRUE(decision.standConnectionStatus.has_value());
    EXPECT_EQ(*decision.standConnectionStatus, domain::StandConnectionStatus::Connecting);
    EXPECT_FALSE(decision.telemetryStatus.has_value());
    EXPECT_FALSE(decision.shouldStartPolling);
}

TEST(TelemetryConnectionPolicyTest, ConnectingStatus_WhenDisconnectingIsActive_DoesNotChangeState) {
    const auto decision = domain::TelemetryConnectionPolicy::handleStatus(
        domain::StandConnectionStatus::Disconnecting, domain::TelemetryConnectionStatus::Connecting, false);

    EXPECT_FALSE(decision.standConnectionStatus.has_value());
    EXPECT_FALSE(decision.telemetryStatus.has_value());
    EXPECT_FALSE(decision.shouldStartPolling);
}

TEST(TelemetryConnectionPolicyTest, ConnectedStatus_WhenTestIsActive_StartsPolling) {
    const auto decision = domain::TelemetryConnectionPolicy::handleStatus(
        domain::StandConnectionStatus::Connecting, domain::TelemetryConnectionStatus::Connected, true);

    ASSERT_TRUE(decision.standConnectionStatus.has_value());
    ASSERT_TRUE(decision.telemetryStatus.has_value());
    EXPECT_EQ(*decision.standConnectionStatus, domain::StandConnectionStatus::Polling);
    EXPECT_EQ(*decision.telemetryStatus, domain::TelemetryStatus::Valid);
    EXPECT_TRUE(decision.shouldStartPolling);
}

TEST(TelemetryConnectionPolicyTest, ConnectedStatus_WhenAlreadyPolling_DoesNotDowngradeStandState) {
    const auto decision = domain::TelemetryConnectionPolicy::handleStatus(
        domain::StandConnectionStatus::Polling, domain::TelemetryConnectionStatus::Connected, true);

    EXPECT_FALSE(decision.standConnectionStatus.has_value());
    ASSERT_TRUE(decision.telemetryStatus.has_value());
    EXPECT_EQ(*decision.telemetryStatus, domain::TelemetryStatus::Valid);
    EXPECT_FALSE(decision.shouldStartPolling);
}

TEST(TelemetryConnectionPolicyTest, ConnectedStatus_WhenNoTestIsActive_MarksStandConnected) {
    const auto decision = domain::TelemetryConnectionPolicy::handleStatus(
        domain::StandConnectionStatus::Connecting, domain::TelemetryConnectionStatus::Connected, false);

    ASSERT_TRUE(decision.standConnectionStatus.has_value());
    ASSERT_TRUE(decision.telemetryStatus.has_value());
    EXPECT_EQ(*decision.standConnectionStatus, domain::StandConnectionStatus::Connected);
    EXPECT_EQ(*decision.telemetryStatus, domain::TelemetryStatus::Valid);
    EXPECT_FALSE(decision.shouldStartPolling);
}

TEST(TelemetryConnectionPolicyTest, DisconnectedStatus_WhenConnectionWasActive_MarksFailure) {
    const auto decision = domain::TelemetryConnectionPolicy::handleStatus(
        domain::StandConnectionStatus::Connected, domain::TelemetryConnectionStatus::Disconnected, false);

    ASSERT_TRUE(decision.standConnectionStatus.has_value());
    ASSERT_TRUE(decision.telemetryStatus.has_value());
    EXPECT_EQ(*decision.standConnectionStatus, domain::StandConnectionStatus::Error);
    EXPECT_EQ(*decision.telemetryStatus, domain::TelemetryStatus::Unavailable);
}

TEST(TelemetryConnectionPolicyTest, ErrorStatus_MarksFailure) {
    const auto decision = domain::TelemetryConnectionPolicy::failure();

    ASSERT_TRUE(decision.standConnectionStatus.has_value());
    ASSERT_TRUE(decision.telemetryStatus.has_value());
    EXPECT_EQ(*decision.standConnectionStatus, domain::StandConnectionStatus::Error);
    EXPECT_EQ(*decision.telemetryStatus, domain::TelemetryStatus::Unavailable);
    EXPECT_FALSE(decision.shouldStartPolling);
}

} // namespace
