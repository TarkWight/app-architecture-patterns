#ifndef STANDCONNECTIONSTATUS_HPP
#define STANDCONNECTIONSTATUS_HPP

namespace domain {

enum class StandConnectionStatus {
    Disconnected,
    Configured,
    Connecting,
    Connected,
    Polling,
    Disconnecting,
    Error,
};

} // namespace domain

#endif // STANDCONNECTIONSTATUS_HPP
