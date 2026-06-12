#include "DisconnectStandUseCase.hpp"

#include "../../Domain/StandConnectionStatus.hpp"

#include <QtCore/QLoggingCategory>

namespace application::useCases {

namespace {

Q_LOGGING_CATEGORY(logUseCase, "mvp.usecase")

QString standConnectionStatusName(domain::StandConnectionStatus status) {
    switch (status) {
        case domain::StandConnectionStatus::Disconnected:
            return QStringLiteral("Disconnected");
        case domain::StandConnectionStatus::Configured:
            return QStringLiteral("Configured");
        case domain::StandConnectionStatus::Connecting:
            return QStringLiteral("Connecting");
        case domain::StandConnectionStatus::Connected:
            return QStringLiteral("Connected");
        case domain::StandConnectionStatus::Polling:
            return QStringLiteral("Polling");
        case domain::StandConnectionStatus::Disconnecting:
            return QStringLiteral("Disconnecting");
        case domain::StandConnectionStatus::Error:
            return QStringLiteral("Error");
    }

    return QStringLiteral("Unknown");
}

} // namespace

DisconnectStandUseCase::DisconnectStandUseCase(application::session::SessionState &state,
                                               application::ports::ITelemetryClient &telemetryClient)
    : state(state), telemetryClient(telemetryClient) {
}

void DisconnectStandUseCase::execute() {
    const auto currentStatus = state.get().standConnectionStatus;

    qCInfo(logUseCase) << "DisconnectStandUseCase execute"
                       << "currentStatus=" << standConnectionStatusName(currentStatus);

    if (currentStatus == domain::StandConnectionStatus::Disconnected) {
        qCInfo(logUseCase) << "DisconnectStandUseCase skipped"
                           << "reason=already_disconnected";
        return;
    }

    qCInfo(logUseCase) << "DisconnectStandUseCase status transition"
                       << standConnectionStatusName(currentStatus)
                       << "->"
                       << standConnectionStatusName(domain::StandConnectionStatus::Disconnecting);

    state.setStandConnectionStatus(domain::StandConnectionStatus::Disconnecting);

    qCInfo(logUseCase) << "DisconnectStandUseCase telemetryClient.stopPolling";
    telemetryClient.stopPolling();

    qCInfo(logUseCase) << "DisconnectStandUseCase telemetryClient.disconnectAll";
    telemetryClient.disconnectAll();

    qCInfo(logUseCase) << "DisconnectStandUseCase status transition"
                       << standConnectionStatusName(domain::StandConnectionStatus::Disconnecting)
                       << "->"
                       << standConnectionStatusName(domain::StandConnectionStatus::Disconnected);

    state.setStandConnectionStatus(domain::StandConnectionStatus::Disconnected);
}

} // namespace application::useCases