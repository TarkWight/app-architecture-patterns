#include "QtTcpTelemetryClient.hpp"

#include <QDateTime>

namespace infrastructure::axisTcp {

QtTcpTelemetryClient::QtTcpTelemetryClient(
    application::ports::IAxisProtocolCodec &codec,
    QObject *parent)
    : QObject(parent),
      codec(codec) {
    QObject::connect(&pollTimer, &QTimer::timeout, this, &QtTcpTelemetryClient::handlePollTimer);
}

QtTcpTelemetryClient::~QtTcpTelemetryClient() {
    stopPolling();
    disconnectAll();
}

void QtTcpTelemetryClient::setTelemetryCallback(TelemetryCallback callback) {
    telemetryCallback = std::move(callback);
}

void QtTcpTelemetryClient::setStatusCallback(StatusCallback callback) {
    statusCallback = std::move(callback);
}

void QtTcpTelemetryClient::setErrorCallback(ErrorCallback callback) {
    errorCallback = std::move(callback);
}

void QtTcpTelemetryClient::configureAxis(domain::AxisId axisId, std::string host, int port) {
    auto &connection = axes[axisId];

    connection.host = QString::fromStdString(host);
    connection.port = port;
    connection.configured = true;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, "Axis configured");
}

void QtTcpTelemetryClient::connectAxis(domain::AxisId axisId) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        emitError(axisId, "Axis is not configured");
        return;
    }

    if (connection->host.isEmpty() || connection->port <= 0) {
        emitError(axisId, "Axis endpoint is invalid");
        return;
    }

    createSocketIfNeeded(axisId, *connection);

    if (connection->socket->state() == QAbstractSocket::ConnectedState ||
        connection->socket->state() == QAbstractSocket::ConnectingState) {
        return;
    }

    connection->readBuffer.clear();
    connection->requestPending = false;
    connection->lastReconnectAttemptMs = nowMs();

    emitStatus(axisId, domain::TelemetryConnectionStatus::Connecting, "Connecting");

    connection->socket->connectToHost(connection->host, static_cast<quint16>(connection->port));
}

void QtTcpTelemetryClient::disconnectAxis(domain::AxisId axisId) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr || connection->socket == nullptr) {
        return;
    }

    connection->socket->disconnectFromHost();
    connection->readBuffer.clear();
    connection->requestPending = false;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, "Disconnected");
}

void QtTcpTelemetryClient::connectAll() {
    for (auto &[axisId, connection] : axes) {
        if (!connection.configured) {
            continue;
        }

        connectAxis(axisId);
    }
}

void QtTcpTelemetryClient::disconnectAll() {
    for (auto &[axisId, connection] : axes) {
        if (connection.socket == nullptr) {
            continue;
        }

        connection.socket->disconnectFromHost();
        connection.readBuffer.clear();
        connection.requestPending = false;

        emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, "Disconnected");
    }
}

void QtTcpTelemetryClient::startPolling(int intervalMs) {
    pollingIntervalMs = intervalMs > 0 ? intervalMs : 1000;
    pollTimer.setInterval(pollingIntervalMs);
    pollTimer.start();

    for (const auto &[axisId, connection] : axes) {
        if (connection.socket != nullptr && connection.socket->state() == QAbstractSocket::ConnectedState) {
            emitStatus(axisId, domain::TelemetryConnectionStatus::Polling, "Polling started");
        }
    }
}

void QtTcpTelemetryClient::stopPolling() {
    pollTimer.stop();

    for (auto &[axisId, connection] : axes) {
        connection.requestPending = false;

        if (connection.socket != nullptr && connection.socket->state() == QAbstractSocket::ConnectedState) {
            emitStatus(axisId, domain::TelemetryConnectionStatus::Connected, "Polling stopped");
        }
    }
}

void QtTcpTelemetryClient::setAxisCommand(domain::AxisId axisId, domain::AxisControlCommand command) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        emitError(axisId, "Axis is not configured");
        return;
    }

    connection->command = command;
}

void QtTcpTelemetryClient::pollOnce(domain::AxisId axisId) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        emitError(axisId, "Axis is not configured");
        return;
    }

    if (connection->socket == nullptr || connection->socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    if (connection->requestPending) {
        return;
    }

    const auto packet = codec.encodeCommand(connection->command);
    if (packet.empty()) {
        emitError(axisId, "Failed to encode axis command");
        return;
    }

    const auto bytesWritten = connection->socket->write(
        reinterpret_cast<const char *>(packet.data()),
        static_cast<qint64>(packet.size()));

    if (bytesWritten != static_cast<qint64>(packet.size())) {
        emitError(axisId, "Failed to write full command packet");
        return;
    }

    connection->requestPending = true;
    connection->lastRequestMs = nowMs();

    emitStatus(axisId, domain::TelemetryConnectionStatus::Polling, "Command sent");
}

QtTcpTelemetryClient::AxisConnection *QtTcpTelemetryClient::findAxis(domain::AxisId axisId) {
    const auto connectionIt = axes.find(axisId);
    if (connectionIt == axes.end()) {
        return nullptr;
    }

    return &connectionIt->second;
}

void QtTcpTelemetryClient::createSocketIfNeeded(domain::AxisId axisId, AxisConnection &connection) {
    if (connection.socket != nullptr) {
        return;
    }

    connection.socket = std::make_unique<QTcpSocket>();

    QObject::connect(connection.socket.get(), &QTcpSocket::connected, this, [this, axisId]() {
        handleConnected(axisId);
    });

    QObject::connect(connection.socket.get(), &QTcpSocket::disconnected, this, [this, axisId]() {
        handleDisconnected(axisId);
    });

    QObject::connect(connection.socket.get(), &QTcpSocket::readyRead, this, [this, axisId]() {
        handleReadyRead(axisId);
    });

    QObject::connect(
        connection.socket.get(),
        &QTcpSocket::errorOccurred,
        this,
        [this, axisId](QAbstractSocket::SocketError error) {
            handleSocketError(axisId, error);
        });
}

void QtTcpTelemetryClient::handlePollTimer() {
    handlePendingTimeouts();
    reconnectDisconnectedAxes();

    for (const auto &[axisId, connection] : axes) {
        if (!connection.configured) {
            continue;
        }

        pollOnce(axisId);
    }
}

void QtTcpTelemetryClient::handleConnected(domain::AxisId axisId) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        return;
    }

    connection->readBuffer.clear();
    connection->requestPending = false;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Connected, "Connected");
}

void QtTcpTelemetryClient::handleDisconnected(domain::AxisId axisId) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        return;
    }

    connection->readBuffer.clear();
    connection->requestPending = false;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, "Disconnected");
}

void QtTcpTelemetryClient::handleSocketError(domain::AxisId axisId, QAbstractSocket::SocketError /*error*/) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr || connection->socket == nullptr) {
        return;
    }

    connection->readBuffer.clear();
    connection->requestPending = false;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Error, connection->socket->errorString().toStdString());
    emitError(axisId, connection->socket->errorString().toStdString());
}

void QtTcpTelemetryClient::handleReadyRead(domain::AxisId axisId) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr || connection->socket == nullptr) {
        return;
    }

    connection->readBuffer.append(connection->socket->readAll());
    processReadBuffer(axisId, *connection);
}

void QtTcpTelemetryClient::processReadBuffer(domain::AxisId axisId, AxisConnection &connection) {
    while (connection.readBuffer.size() >= expectedTelemetryResponseSize) {
        const QByteArray frame = connection.readBuffer.left(expectedTelemetryResponseSize);
        connection.readBuffer.remove(0, expectedTelemetryResponseSize);

        std::vector<std::uint8_t> bytes{};
        bytes.reserve(static_cast<std::size_t>(frame.size()));

        for (const auto byte : frame) {
            bytes.push_back(static_cast<std::uint8_t>(byte));
        }

        const auto sample = codec.decodeTelemetry(axisId, bytes, nowSeconds());
        if (!sample.has_value()) {
            emitError(axisId, "Failed to decode telemetry frame");
            continue;
        }

        connection.requestPending = false;

        if (telemetryCallback) {
            telemetryCallback(*sample);
        }
    }
}

void QtTcpTelemetryClient::handlePendingTimeouts() {
    const qint64 currentMs = nowMs();

    for (auto &[axisId, connection] : axes) {
        if (!connection.requestPending) {
            continue;
        }

        if ((currentMs - connection.lastRequestMs) < responseTimeoutMs) {
            continue;
        }

        connection.requestPending = false;
        connection.readBuffer.clear();

        emitError(axisId, "Telemetry response timeout");

        if (connection.socket != nullptr) {
            connection.socket->abort();
        }
    }
}

void QtTcpTelemetryClient::reconnectDisconnectedAxes() {
    const qint64 currentMs = nowMs();

    for (auto &[axisId, connection] : axes) {
        if (!connection.configured) {
            continue;
        }

        if (connection.socket != nullptr &&
            (connection.socket->state() == QAbstractSocket::ConnectedState ||
             connection.socket->state() == QAbstractSocket::ConnectingState)) {
            continue;
        }

        if ((currentMs - connection.lastReconnectAttemptMs) < reconnectIntervalMs) {
            continue;
        }

        connectAxis(axisId);
    }
}

void QtTcpTelemetryClient::emitStatus(
    domain::AxisId axisId,
    domain::TelemetryConnectionStatus status,
    const std::string &message) {
    auto *connection = findAxis(axisId);
    if (connection != nullptr) {
        connection->status = status;
    }

    if (statusCallback) {
        statusCallback(axisId, status, message);
    }
}

void QtTcpTelemetryClient::emitError(domain::AxisId axisId, const std::string &message) {
    if (errorCallback) {
        errorCallback(axisId, message);
    }
}

qint64 QtTcpTelemetryClient::nowMs() {
    return QDateTime::currentMSecsSinceEpoch();
}

double QtTcpTelemetryClient::nowSeconds() {
    return static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000.0;
}

} // namespace infrastructure::axisTcp