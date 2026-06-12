#include "ConnectStandUseCase.hpp"

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

ConnectStandUseCase::ConnectStandUseCase(application::session::SessionState &state,
                                         application::ports::ITelemetryClient &telemetryClient)
    : state(state), telemetryClient(telemetryClient) {
}

void ConnectStandUseCase::execute() {
    const auto currentStatus = state.get().standConnectionStatus;

    qCInfo(logUseCase) << "ConnectStandUseCase execute"
                       << "currentStatus=" << standConnectionStatusName(currentStatus);

    if (currentStatus == domain::StandConnectionStatus::Connecting ||
        currentStatus == domain::StandConnectionStatus::Connected ||
        currentStatus == domain::StandConnectionStatus::Polling) {
        qCInfo(logUseCase) << "ConnectStandUseCase skipped"
                           << "reason=already_active"
                           << "currentStatus=" << standConnectionStatusName(currentStatus);
        return;
    }

    qCInfo(logUseCase) << "ConnectStandUseCase status transition"
                       << standConnectionStatusName(currentStatus)
                       << "->"
                       << standConnectionStatusName(domain::StandConnectionStatus::Connecting);

    state.setStandConnectionStatus(domain::StandConnectionStatus::Connecting);

    qCInfo(logUseCase) << "ConnectStandUseCase telemetryClient.connectAll";
    telemetryClient.connectAll();
}

} // namespace application::useCases