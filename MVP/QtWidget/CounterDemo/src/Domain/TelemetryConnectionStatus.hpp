#ifndef TELEMETRYCONNECTIONSTATUS_HPP
#define TELEMETRYCONNECTIONSTATUS_HPP

namespace domain {

enum class TelemetryConnectionStatus {
    Disconnected,
    Connecting,
    Connected,
    Polling,
    Error,
};

} // namespace domain

#endif // TELEMETRYCONNECTIONSTATUS_HPP