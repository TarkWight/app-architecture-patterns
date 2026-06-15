#ifndef STANDCONNECTIONTRANSITIONS_HPP
#define STANDCONNECTIONTRANSITIONS_HPP

#include "StandConnectionStatus.hpp"

namespace domain {

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

} // namespace domain

#endif // STANDCONNECTIONTRANSITIONS_HPP
