#include "QtTcpTelemetryClient.hpp"

#include "StandTraceFormatter.hpp"
#include "../../Localization/TransportStrings.hpp"

#include <QDateTime>
#include <QThread>

namespace infrastructure::axisTcp {

namespace {

void assertCurrentThreadOwns(const QObject &object) {
    Q_ASSERT(object.thread() == QThread::currentThread());
}

} // namespace

QtTcpTelemetryClient::QtTcpTelemetryClient(application::ports::IAxisProtocolCodec &codec, QObject *parent)
    : QObject(parent), codec(codec) {
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

void QtTcpTelemetryClient::setTraceCallback(TraceCallback callback) {
    traceCallback = std::move(callback);
}

void QtTcpTelemetryClient::configureAxis(domain::AxisId axisId, std::string host, int port) {
    assertCurrentThreadOwns(*this);

    auto &connection = axes[axisId];

    connection.host = QString::fromStdString(host);
    connection.port = port;
    connection.configured = true;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, localization::transport::axisConfigured);
    emitTrace(formatStandLifecycleTrace("CONFIG", axisId,
                                        connection.host.toStdString() + ":" + std::to_string(connection.port)));
}

void QtTcpTelemetryClient::connectAxis(domain::AxisId axisId) {
    assertCurrentThreadOwns(*this);

    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        emitError(axisId, localization::transport::axisNotConfigured);
        return;
    }

    if (connection->host.isEmpty() || connection->port <= 0) {
        emitError(axisId, localization::transport::axisEndpointInvalid);
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

    emitStatus(axisId, domain::TelemetryConnectionStatus::Connecting, localization::transport::connecting);
    emitTrace(formatStandLifecycleTrace("CONNECT", axisId,
                                        connection->host.toStdString() + ":" + std::to_string(connection->port)));

    connection->socket->connectToHost(connection->host, static_cast<quint16>(connection->port));
}

void QtTcpTelemetryClient::disconnectAxis(domain::AxisId axisId) {
    assertCurrentThreadOwns(*this);

    auto *connection = findAxis(axisId);
    if (connection == nullptr || connection->socket == nullptr) {
        return;
    }

    connection->socket->disconnectFromHost();
    connection->readBuffer.clear();
    connection->requestPending = false;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, localization::transport::disconnected);
    emitTrace(formatStandLifecycleTrace("DISCONNECT", axisId, localization::transport::disconnectRequested));
}

void QtTcpTelemetryClient::connectAll() {
    assertCurrentThreadOwns(*this);

    for (auto &[axisId, connection] : axes) {
        if (!connection.configured) {
            continue;
        }

        connectAxis(axisId);
    }
}

void QtTcpTelemetryClient::disconnectAll() {
    assertCurrentThreadOwns(*this);

    pollTimer.stop();

    for (auto &[axisId, connection] : axes) {
        if (connection.socket == nullptr) {
            continue;
        }

        connection.socket->disconnectFromHost();
        connection.readBuffer.clear();
        connection.requestPending = false;

        emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, localization::transport::disconnected);
        emitTrace(formatStandLifecycleTrace("DISCONNECT", axisId, localization::transport::disconnectRequested));
    }
}

void QtTcpTelemetryClient::startPolling(int intervalMs) {
    assertCurrentThreadOwns(*this);

    pollingIntervalMs = intervalMs > 0 ? intervalMs : 1000;
    pollTimer.setInterval(pollingIntervalMs);
    pollTimer.start();

    for (const auto &[axisId, connection] : axes) {
        if (connection.socket != nullptr && connection.socket->state() == QAbstractSocket::ConnectedState) {
            emitStatus(axisId, domain::TelemetryConnectionStatus::Polling, localization::transport::pollingStarted);
            emitTrace(formatStandLifecycleTrace("POLLING", axisId, localization::transport::pollingStarted));
        }
    }
}

void QtTcpTelemetryClient::stopPolling() {
    assertCurrentThreadOwns(*this);

    pollTimer.stop();

    for (auto &[axisId, connection] : axes) {
        connection.requestPending = false;

        if (connection.socket != nullptr && connection.socket->state() == QAbstractSocket::ConnectedState) {
            emitStatus(axisId, domain::TelemetryConnectionStatus::Connected, localization::transport::pollingStopped);
            emitTrace(formatStandLifecycleTrace("POLLING", axisId, localization::transport::pollingStopped));
        }
    }
}

void QtTcpTelemetryClient::setAxisCommand(domain::AxisId axisId, domain::AxisControlCommand command) {
    assertCurrentThreadOwns(*this);

    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        emitError(axisId, localization::transport::axisNotConfigured);
        return;
    }

    connection->command = command;
}

void QtTcpTelemetryClient::pollOnce(domain::AxisId axisId) {
    assertCurrentThreadOwns(*this);

    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        emitError(axisId, localization::transport::axisNotConfigured);
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
        emitError(axisId, localization::transport::axisCommandEncodeFailed);
        return;
    }

    const auto bytesWritten =
        connection->socket->write(reinterpret_cast<const char *>(packet.data()), static_cast<qint64>(packet.size()));

    if (bytesWritten != static_cast<qint64>(packet.size())) {
        emitError(axisId, localization::transport::axisCommandWriteFailed);
        return;
    }

    connection->requestPending = true;
    connection->lastRequestMs = nowMs();

    emitStatus(axisId, domain::TelemetryConnectionStatus::Polling, localization::transport::commandSent);
    emitTrace(formatStandTxTrace(axisId, connection->command));
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

    QObject::connect(connection.socket.get(), &QTcpSocket::connected, this,
                     [this, axisId]() { handleConnected(axisId); });

    QObject::connect(connection.socket.get(), &QTcpSocket::disconnected, this,
                     [this, axisId]() { handleDisconnected(axisId); });

    QObject::connect(connection.socket.get(), &QTcpSocket::readyRead, this,
                     [this, axisId]() { handleReadyRead(axisId); });

    QObject::connect(connection.socket.get(), &QTcpSocket::errorOccurred, this,
                     [this, axisId](QAbstractSocket::SocketError error) { handleSocketError(axisId, error); });
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

    emitStatus(axisId, domain::TelemetryConnectionStatus::Connected, localization::transport::connected);
    emitTrace(formatStandLifecycleTrace("CONNECTED", axisId));
}

void QtTcpTelemetryClient::handleDisconnected(domain::AxisId axisId) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        return;
    }

    connection->readBuffer.clear();
    connection->requestPending = false;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, localization::transport::disconnected);
    emitTrace(formatStandLifecycleTrace("DISCONNECTED", axisId));
}

void QtTcpTelemetryClient::handleSocketError(domain::AxisId axisId, QAbstractSocket::SocketError /*error*/) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr || connection->socket == nullptr) {
        return;
    }

    connection->readBuffer.clear();
    connection->requestPending = false;

    const auto message = localization::transport::socketError(connection->socket->errorString().toStdString());
    emitStatus(axisId, domain::TelemetryConnectionStatus::Error, message);
    emitError(axisId, message);
    emitTrace(formatStandLifecycleTrace("ERROR", axisId, message));
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
            emitError(axisId, localization::transport::telemetryDecodeFailed);
            continue;
        }

        connection.requestPending = false;
        emitTrace(formatStandRxTrace(*sample));

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

        emitError(axisId, localization::transport::telemetryResponseTimeout);
        emitTrace(formatStandLifecycleTrace("ERROR", axisId, localization::transport::telemetryResponseTimeout));

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

        if (connection.socket != nullptr && (connection.socket->state() == QAbstractSocket::ConnectedState ||
                                             connection.socket->state() == QAbstractSocket::ConnectingState)) {
            continue;
        }

        if ((currentMs - connection.lastReconnectAttemptMs) < reconnectIntervalMs) {
            continue;
        }

        connectAxis(axisId);
    }
}

void QtTcpTelemetryClient::emitStatus(domain::AxisId axisId, domain::TelemetryConnectionStatus status,
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

void QtTcpTelemetryClient::emitTrace(const std::string &message) {
    if (traceCallback) {
        traceCallback(message);
    }
}

qint64 QtTcpTelemetryClient::nowMs() {
    return QDateTime::currentMSecsSinceEpoch();
}

double QtTcpTelemetryClient::nowSeconds() {
    return static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000.0;
}

} // namespace infrastructure::axisTcp
