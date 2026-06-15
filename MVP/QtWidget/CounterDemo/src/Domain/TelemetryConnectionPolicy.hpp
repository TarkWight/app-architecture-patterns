#ifndef TELEMETRYCONNECTIONPOLICY_HPP
#define TELEMETRYCONNECTIONPOLICY_HPP

#include "StandConnectionStatus.hpp"
#include "TelemetryConnectionStatus.hpp"
#include "TelemetryStatus.hpp"

#include <optional>

namespace domain {

struct TelemetryConnectionDecision {
    std::optional<StandConnectionStatus> standConnectionStatus{};
    std::optional<TelemetryStatus> telemetryStatus{};
    bool shouldStartPolling{false};
};

class TelemetryConnectionPolicy final {
  public:
    [[nodiscard]] static TelemetryConnectionDecision handleStatus(StandConnectionStatus currentStatus,
                                                                  TelemetryConnectionStatus telemetryStatus,
                                                                  bool testExecutionIsActive) {
        switch (telemetryStatus) {
        case TelemetryConnectionStatus::Connecting:
            if (currentStatus != StandConnectionStatus::Disconnecting) {
                return TelemetryConnectionDecision{.standConnectionStatus = StandConnectionStatus::Connecting};
            }
            return TelemetryConnectionDecision{};
        case TelemetryConnectionStatus::Connected:
            return connected(currentStatus, testExecutionIsActive);
        case TelemetryConnectionStatus::Polling:
            if (currentStatus != StandConnectionStatus::Disconnecting) {
                return TelemetryConnectionDecision{.standConnectionStatus = StandConnectionStatus::Polling,
                                                   .telemetryStatus = TelemetryStatus::Valid};
            }
            return TelemetryConnectionDecision{};
        case TelemetryConnectionStatus::Disconnected:
            return disconnected(currentStatus);
        case TelemetryConnectionStatus::Error:
            return failure();
        }

        return TelemetryConnectionDecision{};
    }

    [[nodiscard]] static TelemetryConnectionDecision failure() {
        return TelemetryConnectionDecision{.standConnectionStatus = StandConnectionStatus::Error,
                                           .telemetryStatus = TelemetryStatus::Unavailable};
    }

  private:
    [[nodiscard]] static TelemetryConnectionDecision connected(StandConnectionStatus currentStatus,
                                                               bool testExecutionIsActive) {
        if (currentStatus == StandConnectionStatus::Disconnecting) {
            return TelemetryConnectionDecision{};
        }

        if (currentStatus == StandConnectionStatus::Polling) {
            return TelemetryConnectionDecision{.telemetryStatus = TelemetryStatus::Valid};
        }

        if (testExecutionIsActive) {
            return TelemetryConnectionDecision{.standConnectionStatus = StandConnectionStatus::Polling,
                                               .telemetryStatus = TelemetryStatus::Valid,
                                               .shouldStartPolling = true};
        }

        return TelemetryConnectionDecision{.standConnectionStatus = StandConnectionStatus::Connected,
                                           .telemetryStatus = TelemetryStatus::Valid};
    }

    [[nodiscard]] static TelemetryConnectionDecision disconnected(StandConnectionStatus currentStatus) {
        switch (currentStatus) {
        case StandConnectionStatus::Connecting:
        case StandConnectionStatus::Connected:
        case StandConnectionStatus::Polling:
            return failure();
        case StandConnectionStatus::Disconnected:
        case StandConnectionStatus::Configured:
        case StandConnectionStatus::Disconnecting:
        case StandConnectionStatus::Error:
            return TelemetryConnectionDecision{};
        }

        return TelemetryConnectionDecision{};
    }
};

} // namespace domain

#endif // TELEMETRYCONNECTIONPOLICY_HPP
