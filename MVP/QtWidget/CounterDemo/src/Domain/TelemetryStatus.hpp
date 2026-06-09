#ifndef TELEMETRYSTATUS_HPP
#define TELEMETRYSTATUS_HPP

namespace domain {

enum class TelemetryStatus {
    Unavailable,
    Valid,
    Stale,
    Invalid,
};

} // namespace domain
#endif // TELEMETRYSTATUS_HPP
