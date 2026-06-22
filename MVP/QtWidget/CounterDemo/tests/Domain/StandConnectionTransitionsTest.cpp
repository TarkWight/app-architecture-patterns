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

TEST(StandConnectionTransitionsTest, ConnectRequestTransitionsOnlyAllowedStatesToConnecting) {
    EXPECT_EQ(domain::transitionToConnecting(domain::StandConnectionStatus::Disconnected),
              domain::StandConnectionStatus::Connecting);
    EXPECT_EQ(domain::transitionToConnecting(domain::StandConnectionStatus::Configured),
              domain::StandConnectionStatus::Connecting);
    EXPECT_EQ(domain::transitionToConnecting(domain::StandConnectionStatus::Disconnecting),
              domain::StandConnectionStatus::Connecting);
    EXPECT_EQ(domain::transitionToConnecting(domain::StandConnectionStatus::Error),
              domain::StandConnectionStatus::Connecting);

    EXPECT_FALSE(domain::transitionToConnecting(domain::StandConnectionStatus::Connecting).has_value());
    EXPECT_FALSE(domain::transitionToConnecting(domain::StandConnectionStatus::Connected).has_value());
    EXPECT_FALSE(domain::transitionToConnecting(domain::StandConnectionStatus::Polling).has_value());
}

TEST(StandConnectionTransitionsTest, DisconnectRequestTransitionsEveryConnectedStateToDisconnecting) {
    EXPECT_FALSE(domain::transitionToDisconnecting(domain::StandConnectionStatus::Disconnected).has_value());
    EXPECT_EQ(domain::transitionToDisconnecting(domain::StandConnectionStatus::Configured),
              domain::StandConnectionStatus::Disconnecting);
    EXPECT_EQ(domain::transitionToDisconnecting(domain::StandConnectionStatus::Connecting),
              domain::StandConnectionStatus::Disconnecting);
    EXPECT_EQ(domain::transitionToDisconnecting(domain::StandConnectionStatus::Connected),
              domain::StandConnectionStatus::Disconnecting);
    EXPECT_EQ(domain::transitionToDisconnecting(domain::StandConnectionStatus::Polling),
              domain::StandConnectionStatus::Disconnecting);
    EXPECT_EQ(domain::transitionToDisconnecting(domain::StandConnectionStatus::Disconnecting),
              domain::StandConnectionStatus::Disconnecting);
    EXPECT_EQ(domain::transitionToDisconnecting(domain::StandConnectionStatus::Error),
              domain::StandConnectionStatus::Disconnecting);
}

TEST(StandConnectionTransitionsTest, DisconnectCompletionTransitionsOnlyDisconnectingToDisconnected) {
    EXPECT_FALSE(domain::transitionAfterDisconnectCompleted(domain::StandConnectionStatus::Disconnected).has_value());
    EXPECT_FALSE(domain::transitionAfterDisconnectCompleted(domain::StandConnectionStatus::Configured).has_value());
    EXPECT_FALSE(domain::transitionAfterDisconnectCompleted(domain::StandConnectionStatus::Connecting).has_value());
    EXPECT_FALSE(domain::transitionAfterDisconnectCompleted(domain::StandConnectionStatus::Connected).has_value());
    EXPECT_FALSE(domain::transitionAfterDisconnectCompleted(domain::StandConnectionStatus::Polling).has_value());
    EXPECT_EQ(domain::transitionAfterDisconnectCompleted(domain::StandConnectionStatus::Disconnecting),
              domain::StandConnectionStatus::Disconnected);
    EXPECT_FALSE(domain::transitionAfterDisconnectCompleted(domain::StandConnectionStatus::Error).has_value());
}

TEST(StandConnectionTransitionsTest, PollingStartTransitionsOnlyConnectedToPolling) {
    EXPECT_FALSE(domain::transitionAfterPollingStarted(domain::StandConnectionStatus::Disconnected).has_value());
    EXPECT_FALSE(domain::transitionAfterPollingStarted(domain::StandConnectionStatus::Configured).has_value());
    EXPECT_FALSE(domain::transitionAfterPollingStarted(domain::StandConnectionStatus::Connecting).has_value());
    EXPECT_EQ(domain::transitionAfterPollingStarted(domain::StandConnectionStatus::Connected),
              domain::StandConnectionStatus::Polling);
    EXPECT_FALSE(domain::transitionAfterPollingStarted(domain::StandConnectionStatus::Polling).has_value());
    EXPECT_FALSE(domain::transitionAfterPollingStarted(domain::StandConnectionStatus::Disconnecting).has_value());
    EXPECT_FALSE(domain::transitionAfterPollingStarted(domain::StandConnectionStatus::Error).has_value());
}

TEST(StandConnectionTransitionsTest, PollingStopTransitionsOnlyPollingToConnected) {
    EXPECT_FALSE(domain::transitionAfterPollingStopped(domain::StandConnectionStatus::Disconnected).has_value());
    EXPECT_FALSE(domain::transitionAfterPollingStopped(domain::StandConnectionStatus::Configured).has_value());
    EXPECT_FALSE(domain::transitionAfterPollingStopped(domain::StandConnectionStatus::Connecting).has_value());
    EXPECT_FALSE(domain::transitionAfterPollingStopped(domain::StandConnectionStatus::Connected).has_value());
    EXPECT_EQ(domain::transitionAfterPollingStopped(domain::StandConnectionStatus::Polling),
              domain::StandConnectionStatus::Connected);
    EXPECT_FALSE(domain::transitionAfterPollingStopped(domain::StandConnectionStatus::Disconnecting).has_value());
    EXPECT_FALSE(domain::transitionAfterPollingStopped(domain::StandConnectionStatus::Error).has_value());
}

TEST(StandConnectionTransitionsTest, TelemetryConfigurationTransitionsToConfigured) {
    EXPECT_EQ(domain::transitionAfterTelemetryConfigured(), domain::StandConnectionStatus::Configured);
}

TEST(StandConnectionTransitionsTest, ConnectionButtonActionMapsStandStatusToOperatorIntent) {
    EXPECT_EQ(domain::connectionButtonAction(domain::StandConnectionStatus::Disconnected),
              domain::StandConnectionButtonAction::ConfigureAndConnect);
    EXPECT_EQ(domain::connectionButtonAction(domain::StandConnectionStatus::Error),
              domain::StandConnectionButtonAction::ConfigureAndConnect);
    EXPECT_EQ(domain::connectionButtonAction(domain::StandConnectionStatus::Configured),
              domain::StandConnectionButtonAction::Connect);

    EXPECT_EQ(domain::connectionButtonAction(domain::StandConnectionStatus::Connecting),
              domain::StandConnectionButtonAction::Disconnect);
    EXPECT_EQ(domain::connectionButtonAction(domain::StandConnectionStatus::Connected),
              domain::StandConnectionButtonAction::Disconnect);
    EXPECT_EQ(domain::connectionButtonAction(domain::StandConnectionStatus::Polling),
              domain::StandConnectionButtonAction::Disconnect);
    EXPECT_EQ(domain::connectionButtonAction(domain::StandConnectionStatus::Disconnecting),
              domain::StandConnectionButtonAction::Disconnect);
}

} // namespace
