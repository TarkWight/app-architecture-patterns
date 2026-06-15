#include "../../src/Domain/StandConnectionTransitions.hpp"

#include <gtest/gtest.h>

namespace {

TEST(StandConnectionTransitionsTest, ConnectIsAllowedFromInactiveOrRecoverableStates) {
    EXPECT_TRUE(domain::canConnect(domain::StandConnectionStatus::Disconnected));
    EXPECT_TRUE(domain::canConnect(domain::StandConnectionStatus::Configured));
    EXPECT_TRUE(domain::canConnect(domain::StandConnectionStatus::Disconnecting));
    EXPECT_TRUE(domain::canConnect(domain::StandConnectionStatus::Error));
}

TEST(StandConnectionTransitionsTest, ConnectIsRejectedFromActiveConnectionStates) {
    EXPECT_FALSE(domain::canConnect(domain::StandConnectionStatus::Connecting));
    EXPECT_FALSE(domain::canConnect(domain::StandConnectionStatus::Connected));
    EXPECT_FALSE(domain::canConnect(domain::StandConnectionStatus::Polling));
}

TEST(StandConnectionTransitionsTest, DisconnectIsAllowedFromEveryStateExceptDisconnected) {
    EXPECT_FALSE(domain::canDisconnect(domain::StandConnectionStatus::Disconnected));
    EXPECT_TRUE(domain::canDisconnect(domain::StandConnectionStatus::Configured));
    EXPECT_TRUE(domain::canDisconnect(domain::StandConnectionStatus::Connecting));
    EXPECT_TRUE(domain::canDisconnect(domain::StandConnectionStatus::Connected));
    EXPECT_TRUE(domain::canDisconnect(domain::StandConnectionStatus::Polling));
    EXPECT_TRUE(domain::canDisconnect(domain::StandConnectionStatus::Disconnecting));
    EXPECT_TRUE(domain::canDisconnect(domain::StandConnectionStatus::Error));
}

TEST(StandConnectionTransitionsTest, PollingCanStartOnlyFromConnectedStand) {
    EXPECT_FALSE(domain::canStartPolling(domain::StandConnectionStatus::Disconnected));
    EXPECT_FALSE(domain::canStartPolling(domain::StandConnectionStatus::Configured));
    EXPECT_FALSE(domain::canStartPolling(domain::StandConnectionStatus::Connecting));
    EXPECT_TRUE(domain::canStartPolling(domain::StandConnectionStatus::Connected));
    EXPECT_FALSE(domain::canStartPolling(domain::StandConnectionStatus::Polling));
    EXPECT_FALSE(domain::canStartPolling(domain::StandConnectionStatus::Disconnecting));
    EXPECT_FALSE(domain::canStartPolling(domain::StandConnectionStatus::Error));
}

TEST(StandConnectionTransitionsTest, PollingCanStopOnlyFromPollingStand) {
    EXPECT_FALSE(domain::canStopPolling(domain::StandConnectionStatus::Disconnected));
    EXPECT_FALSE(domain::canStopPolling(domain::StandConnectionStatus::Configured));
    EXPECT_FALSE(domain::canStopPolling(domain::StandConnectionStatus::Connecting));
    EXPECT_FALSE(domain::canStopPolling(domain::StandConnectionStatus::Connected));
    EXPECT_TRUE(domain::canStopPolling(domain::StandConnectionStatus::Polling));
    EXPECT_FALSE(domain::canStopPolling(domain::StandConnectionStatus::Disconnecting));
    EXPECT_FALSE(domain::canStopPolling(domain::StandConnectionStatus::Error));
}

} // namespace
