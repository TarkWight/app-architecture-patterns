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

} // namespace
