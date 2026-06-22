#ifndef STANDCONNECTIONTRANSITIONS_HPP
#define STANDCONNECTIONTRANSITIONS_HPP

#include "StandConnectionStatus.hpp"

#include <optional>

namespace domain {

enum class StandConnectionButtonAction {
    ConfigureAndConnect,
    Connect,
    Disconnect,
};

constexpr bool canConnect(StandConnectionStatus status) {
    switch (status) {
    case StandConnectionStatus::Disconnected:
    case StandConnectionStatus::Configured:
    case StandConnectionStatus::Disconnecting:
    case StandConnectionStatus::Error:
        return true;
    case StandConnectionStatus::Connecting:
    case StandConnectionStatus::Connected:
    case StandConnectionStatus::Polling:
        return false;
    }
    return false;
}

constexpr bool canDisconnect(StandConnectionStatus status) {
    return status != StandConnectionStatus::Disconnected;
}

constexpr bool canStartPolling(StandConnectionStatus status) {
    return status == StandConnectionStatus::Connected;
}

constexpr bool canStopPolling(StandConnectionStatus status) {
    return status == StandConnectionStatus::Polling;
}

inline std::optional<StandConnectionStatus> transitionToConnecting(StandConnectionStatus status) {
    if (!canConnect(status)) {
        return std::nullopt;
    }

    return StandConnectionStatus::Connecting;
}

inline std::optional<StandConnectionStatus> transitionToDisconnecting(StandConnectionStatus status) {
    if (!canDisconnect(status)) {
        return std::nullopt;
    }

    return StandConnectionStatus::Disconnecting;
}

inline std::optional<StandConnectionStatus> transitionAfterDisconnectCompleted(StandConnectionStatus status) {
    if (status != StandConnectionStatus::Disconnecting) {
        return std::nullopt;
    }

    return StandConnectionStatus::Disconnected;
}

inline std::optional<StandConnectionStatus> transitionAfterPollingStarted(StandConnectionStatus status) {
    if (!canStartPolling(status)) {
        return std::nullopt;
    }

    return StandConnectionStatus::Polling;
}

inline std::optional<StandConnectionStatus> transitionAfterPollingStopped(StandConnectionStatus status) {
    if (!canStopPolling(status)) {
        return std::nullopt;
    }

    return StandConnectionStatus::Connected;
}

constexpr StandConnectionStatus transitionAfterTelemetryConfigured() {
    return StandConnectionStatus::Configured;
}

constexpr StandConnectionButtonAction connectionButtonAction(StandConnectionStatus status) {
    switch (status) {
    case StandConnectionStatus::Disconnected:
    case StandConnectionStatus::Error:
        return StandConnectionButtonAction::ConfigureAndConnect;
    case StandConnectionStatus::Configured:
        return StandConnectionButtonAction::Connect;
    case StandConnectionStatus::Connecting:
    case StandConnectionStatus::Connected:
    case StandConnectionStatus::Polling:
    case StandConnectionStatus::Disconnecting:
        return StandConnectionButtonAction::Disconnect;
    }

    return StandConnectionButtonAction::ConfigureAndConnect;
}

} // namespace domain

#endif // STANDCONNECTIONTRANSITIONS_HPP
