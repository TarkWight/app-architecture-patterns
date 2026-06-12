#include "QtTcpTelemetryClient.hpp"

#include <QByteArray>
#include <QDateTime>
#include <QtCore/QLoggingCategory>
#include <QThread>

namespace infrastructure::axisTcp {

namespace {

Q_LOGGING_CATEGORY(logTcp, "mvp.tcp")
Q_LOGGING_CATEGORY(logCodec, "mvp.codec")

void assertCurrentThreadOwns(const QObject &object) {
    Q_ASSERT(object.thread() == QThread::currentThread());
}

QString axisName(domain::AxisId axisId) {
    return axisId == domain::axis0 ? QStringLiteral("axis0") : QStringLiteral("axis1");
}

QString socketStateName(QAbstractSocket::SocketState state) {
    switch (state) {
        case QAbstractSocket::UnconnectedState:
            return QStringLiteral("Unconnected");
        case QAbstractSocket::HostLookupState:
            return QStringLiteral("HostLookup");
        case QAbstractSocket::ConnectingState:
            return QStringLiteral("Connecting");
        case QAbstractSocket::ConnectedState:
            return QStringLiteral("Connected");
        case QAbstractSocket::BoundState:
            return QStringLiteral("Bound");
        case QAbstractSocket::ClosingState:
            return QStringLiteral("Closing");
        case QAbstractSocket::ListeningState:
            return QStringLiteral("Listening");
    }

    return QStringLiteral("Unknown");
}

QString bytesToHex(const QByteArray &bytes) {
    return QString::fromLatin1(bytes.toHex(' ').toUpper());
}

} // namespace

QtTcpTelemetryClient::QtTcpTelemetryClient(application::ports::IAxisProtocolCodec &codec, QObject *parent)
    : QObject(parent), codec(codec) {
    QObject::connect(&pollTimer, &QTimer::timeout, this, &QtTcpTelemetryClient::handlePollTimer);

    qCInfo(logTcp) << "QtTcpTelemetryClient created";
}

QtTcpTelemetryClient::~QtTcpTelemetryClient() {
    qCInfo(logTcp) << "QtTcpTelemetryClient destroyed";

    stopPolling();
    disconnectAll();
}

void QtTcpTelemetryClient::setTelemetryCallback(TelemetryCallback callback) {
    telemetryCallback = std::move(callback);
    qCDebug(logTcp) << "telemetry callback set";
}

void QtTcpTelemetryClient::setStatusCallback(StatusCallback callback) {
    statusCallback = std::move(callback);
    qCDebug(logTcp) << "status callback set";
}

void QtTcpTelemetryClient::setErrorCallback(ErrorCallback callback) {
    errorCallback = std::move(callback);
    qCDebug(logTcp) << "error callback set";
}

void QtTcpTelemetryClient::configureAxis(domain::AxisId axisId, std::string host, int port) {
    assertCurrentThreadOwns(*this);

    auto &connection = axes[axisId];

    connection.host = QString::fromStdString(host);
    connection.port = port;
    connection.configured = true;

    qCInfo(logTcp) << "configureAxis"
                   << axisName(axisId)
                   << "host=" << connection.host
                   << "port=" << connection.port;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, "Axis configured");
}

void QtTcpTelemetryClient::connectAxis(domain::AxisId axisId) {
    assertCurrentThreadOwns(*this);

    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        qCWarning(logTcp) << "connectAxis rejected"
                          << axisName(axisId)
                          << "reason=not_configured";
        emitError(axisId, "Axis is not configured");
        return;
    }

    if (connection->host.isEmpty() || connection->port <= 0) {
        qCWarning(logTcp) << "connectAxis rejected"
                          << axisName(axisId)
                          << "reason=invalid_endpoint"
                          << "host=" << connection->host
                          << "port=" << connection->port;
        emitError(axisId, "Axis endpoint is invalid");
        return;
    }

    createSocketIfNeeded(axisId, *connection);

    qCInfo(logTcp) << "connectAxis"
                   << axisName(axisId)
                   << "host=" << connection->host
                   << "port=" << connection->port
                   << "state=" << socketStateName(connection->socket->state());

    if (connection->socket->state() == QAbstractSocket::ConnectedState ||
        connection->socket->state() == QAbstractSocket::ConnectingState) {
        qCDebug(logTcp) << "connectAxis skipped"
                        << axisName(axisId)
                        << "state=" << socketStateName(connection->socket->state());
        return;
    }

    connection->readBuffer.clear();
    connection->requestPending = false;
    connection->lastReconnectAttemptMs = nowMs();

    emitStatus(axisId, domain::TelemetryConnectionStatus::Connecting, "Connecting");

    connection->socket->connectToHost(connection->host, static_cast<quint16>(connection->port));
}

void QtTcpTelemetryClient::disconnectAxis(domain::AxisId axisId) {
    assertCurrentThreadOwns(*this);

    auto *connection = findAxis(axisId);
    if (connection == nullptr || connection->socket == nullptr) {
        qCDebug(logTcp) << "disconnectAxis skipped"
                        << axisName(axisId)
                        << "reason=no_socket";
        return;
    }

    qCInfo(logTcp) << "disconnectAxis"
                   << axisName(axisId)
                   << "state=" << socketStateName(connection->socket->state())
                   << "bufferSize=" << connection->readBuffer.size()
                   << "requestPending=" << connection->requestPending;

    connection->socket->disconnectFromHost();
    connection->readBuffer.clear();
    connection->requestPending = false;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, "Disconnected");
}

void QtTcpTelemetryClient::connectAll() {
    assertCurrentThreadOwns(*this);

    qCInfo(logTcp) << "connectAll axisCount=" << axes.size();

    for (auto &[axisId, connection] : axes) {
        qCDebug(logTcp) << "connectAll axis"
                        << axisName(axisId)
                        << "configured=" << connection.configured
                        << "host=" << connection.host
                        << "port=" << connection.port;

        if (!connection.configured) {
            continue;
        }

        connectAxis(axisId);
    }
}

void QtTcpTelemetryClient::disconnectAll() {
    assertCurrentThreadOwns(*this);

    qCInfo(logTcp) << "disconnectAll axisCount=" << axes.size();

    pollTimer.stop();

    for (auto &[axisId, connection] : axes) {
        if (connection.socket == nullptr) {
            qCDebug(logTcp) << "disconnectAll skipped"
                            << axisName(axisId)
                            << "reason=no_socket";
            continue;
        }

        qCInfo(logTcp) << "disconnectAll axis"
                       << axisName(axisId)
                       << "state=" << socketStateName(connection.socket->state())
                       << "bufferSize=" << connection.readBuffer.size()
                       << "requestPending=" << connection.requestPending;

        connection.socket->disconnectFromHost();
        connection.readBuffer.clear();
        connection.requestPending = false;

        emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, "Disconnected");
    }
}

void QtTcpTelemetryClient::startPolling(int intervalMs) {
    assertCurrentThreadOwns(*this);

    pollingIntervalMs = intervalMs > 0 ? intervalMs : 1000;
    pollTimer.setInterval(pollingIntervalMs);
    pollTimer.start();

    qCInfo(logTcp) << "startPolling"
                   << "intervalMs=" << pollingIntervalMs
                   << "axisCount=" << axes.size();

    for (const auto &[axisId, connection] : axes) {
        qCDebug(logTcp) << "startPolling axis"
                        << axisName(axisId)
                        << "configured=" << connection.configured
                        << "socketExists=" << (connection.socket != nullptr)
                        << "state="
                        << (connection.socket != nullptr ? socketStateName(connection.socket->state())
                                                         : QStringLiteral("null"));

        if (connection.socket != nullptr && connection.socket->state() == QAbstractSocket::ConnectedState) {
            emitStatus(axisId, domain::TelemetryConnectionStatus::Polling, "Polling started");
        }
    }
}

void QtTcpTelemetryClient::stopPolling() {
    assertCurrentThreadOwns(*this);

    qCInfo(logTcp) << "stopPolling";

    pollTimer.stop();

    for (auto &[axisId, connection] : axes) {
        qCDebug(logTcp) << "stopPolling axis"
                        << axisName(axisId)
                        << "requestPendingBefore=" << connection.requestPending;

        connection.requestPending = false;

        if (connection.socket != nullptr && connection.socket->state() == QAbstractSocket::ConnectedState) {
            emitStatus(axisId, domain::TelemetryConnectionStatus::Connected, "Polling stopped");
        }
    }
}

void QtTcpTelemetryClient::setAxisCommand(domain::AxisId axisId, domain::AxisControlCommand command) {
    assertCurrentThreadOwns(*this);

    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        qCWarning(logTcp) << "setAxisCommand rejected"
                          << axisName(axisId)
                          << "reason=not_configured";
        emitError(axisId, "Axis is not configured");
        return;
    }

    qCDebug(logTcp) << "setAxisCommand"
                    << axisName(axisId)
                    << "position=" << command.position
                    << "velocity=" << command.velocity
                    << "torque=" << command.torque
                    << "flags=" << command.cmd1 << command.cmd2 << command.cmd3 << command.cmd4;

    connection->command = command;
}

void QtTcpTelemetryClient::pollOnce(domain::AxisId axisId) {
    assertCurrentThreadOwns(*this);

    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        qCWarning(logTcp) << "pollOnce rejected"
                          << axisName(axisId)
                          << "reason=not_configured";
        emitError(axisId, "Axis is not configured");
        return;
    }

    if (connection->socket == nullptr || connection->socket->state() != QAbstractSocket::ConnectedState) {
        qCDebug(logTcp) << "pollOnce skipped"
                        << axisName(axisId)
                        << "reason=not_connected"
                        << "socketExists=" << (connection->socket != nullptr)
                        << "state="
                        << (connection->socket != nullptr ? socketStateName(connection->socket->state())
                                                          : QStringLiteral("null"));
        return;
    }

    if (connection->requestPending) {
        qCDebug(logTcp) << "pollOnce skipped"
                        << axisName(axisId)
                        << "reason=request_pending"
                        << "ageMs=" << (nowMs() - connection->lastRequestMs)
                        << "bufferSize=" << connection->readBuffer.size();
        return;
    }

    const auto packet = codec.encodeCommand(connection->command);
    if (packet.empty()) {
        qCWarning(logTcp) << "pollOnce failed"
                          << axisName(axisId)
                          << "reason=encode_failed";
        emitError(axisId, "Failed to encode axis command");
        return;
    }

    const QByteArray packetBytes(reinterpret_cast<const char *>(packet.data()),
                                 static_cast<qsizetype>(packet.size()));

    qCInfo(logTcp) << "TX"
                   << axisName(axisId)
                   << packetBytes.size() << "bytes"
                   << bytesToHex(packetBytes);

    const auto bytesWritten =
        connection->socket->write(reinterpret_cast<const char *>(packet.data()), static_cast<qint64>(packet.size()));

    qCDebug(logTcp) << "write result"
                    << axisName(axisId)
                    << "bytesWritten=" << bytesWritten
                    << "expected=" << packet.size()
                    << "bytesToWrite=" << connection->socket->bytesToWrite();

    if (bytesWritten != static_cast<qint64>(packet.size())) {
        qCWarning(logTcp) << "pollOnce failed"
                          << axisName(axisId)
                          << "reason=partial_write"
                          << "bytesWritten=" << bytesWritten
                          << "expected=" << packet.size();
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

    qCInfo(logTcp) << "socket created" << axisName(axisId);

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
    qCDebug(logTcp) << "pollTimer timeout";

    handlePendingTimeouts();
    reconnectDisconnectedAxes();

    for (const auto &[axisId, connection] : axes) {
        if (!connection.configured) {
            qCDebug(logTcp) << "pollTimer skip"
                            << axisName(axisId)
                            << "reason=not_configured";
            continue;
        }

        pollOnce(axisId);
    }
}

void QtTcpTelemetryClient::handleConnected(domain::AxisId axisId) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        qCWarning(logTcp) << "handleConnected ignored"
                          << axisName(axisId)
                          << "reason=not_configured";
        return;
    }

    connection->readBuffer.clear();
    connection->requestPending = false;

    qCInfo(logTcp) << "connected"
                   << axisName(axisId)
                   << "state="
                   << (connection->socket != nullptr ? socketStateName(connection->socket->state())
                                                     : QStringLiteral("null"));

    emitStatus(axisId, domain::TelemetryConnectionStatus::Connected, "Connected");
}

void QtTcpTelemetryClient::handleDisconnected(domain::AxisId axisId) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr) {
        qCWarning(logTcp) << "handleDisconnected ignored"
                          << axisName(axisId)
                          << "reason=not_configured";
        return;
    }

    qCWarning(logTcp) << "disconnected"
                      << axisName(axisId)
                      << "bufferSize=" << connection->readBuffer.size()
                      << "requestPending=" << connection->requestPending;

    connection->readBuffer.clear();
    connection->requestPending = false;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Disconnected, "Disconnected");
}

void QtTcpTelemetryClient::handleSocketError(domain::AxisId axisId, QAbstractSocket::SocketError error) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr || connection->socket == nullptr) {
        qCWarning(logTcp) << "handleSocketError ignored"
                          << axisName(axisId)
                          << "reason=no_socket"
                          << "error=" << error;
        return;
    }

    qCWarning(logTcp) << "socket error"
                      << axisName(axisId)
                      << "error=" << error
                      << "message=" << connection->socket->errorString()
                      << "state=" << socketStateName(connection->socket->state())
                      << "bufferSize=" << connection->readBuffer.size()
                      << "requestPending=" << connection->requestPending;

    connection->readBuffer.clear();
    connection->requestPending = false;

    emitStatus(axisId, domain::TelemetryConnectionStatus::Error, connection->socket->errorString().toStdString());
    emitError(axisId, connection->socket->errorString().toStdString());
}

void QtTcpTelemetryClient::handleReadyRead(domain::AxisId axisId) {
    auto *connection = findAxis(axisId);
    if (connection == nullptr || connection->socket == nullptr) {
        qCWarning(logTcp) << "handleReadyRead ignored"
                          << axisName(axisId)
                          << "reason=no_socket";
        return;
    }

    const QByteArray chunk = connection->socket->readAll();

    qCInfo(logTcp) << "RX chunk"
                   << axisName(axisId)
                   << chunk.size() << "bytes"
                   << "bytesAvailableAfterRead=" << connection->socket->bytesAvailable()
                   << bytesToHex(chunk);

    connection->readBuffer.append(chunk);

    qCDebug(logTcp) << "readBuffer"
                    << axisName(axisId)
                    << "size=" << connection->readBuffer.size();

    processReadBuffer(axisId, *connection);
}

void QtTcpTelemetryClient::processReadBuffer(domain::AxisId axisId, AxisConnection &connection) {
    while (connection.readBuffer.size() >= expectedTelemetryResponseSize) {
        const QByteArray frame = connection.readBuffer.left(expectedTelemetryResponseSize);
        connection.readBuffer.remove(0, expectedTelemetryResponseSize);

        qCInfo(logCodec) << "decode frame"
                         << axisName(axisId)
                         << frame.size() << "bytes"
                         << bytesToHex(frame);

        std::vector<std::uint8_t> bytes{};
        bytes.reserve(static_cast<std::size_t>(frame.size()));

        for (const auto byte : frame) {
            bytes.push_back(static_cast<std::uint8_t>(byte));
        }

        const auto sample = codec.decodeTelemetry(axisId, bytes, nowSeconds());
        if (!sample.has_value()) {
            qCWarning(logCodec) << "decode failed"
                                << axisName(axisId)
                                << frame.size() << "bytes"
                                << bytesToHex(frame);

            emitError(axisId, "Failed to decode telemetry frame");
            continue;
        }

        qCInfo(logCodec) << "telemetry decoded"
                         << axisName(axisId)
                         << "position=" << sample->position
                         << "setPosition=" << sample->setPosition
                         << "torque=" << sample->torque
                         << "setTorque=" << sample->setTorque
                         << "voltage=" << sample->voltage
                         << "current=" << sample->current
                         << "valid=" << sample->valid;

        connection.requestPending = false;

        if (telemetryCallback) {
            qCDebug(logTcp) << "telemetry callback"
                            << axisName(axisId);

            telemetryCallback(*sample);
        }
    }

    if (!connection.readBuffer.isEmpty()) {
        qCDebug(logTcp) << "partial RX buffer"
                        << axisName(axisId)
                        << "remaining=" << connection.readBuffer.size()
                        << bytesToHex(connection.readBuffer);
    }
}

void QtTcpTelemetryClient::handlePendingTimeouts() {
    const qint64 currentMs = nowMs();

    for (auto &[axisId, connection] : axes) {
        if (!connection.requestPending) {
            continue;
        }

        const qint64 ageMs = currentMs - connection.lastRequestMs;

        if (ageMs < responseTimeoutMs) {
            qCDebug(logTcp) << "pending request alive"
                            << axisName(axisId)
                            << "ageMs=" << ageMs
                            << "timeoutMs=" << responseTimeoutMs;
            continue;
        }

        qCWarning(logTcp) << "response timeout"
                          << axisName(axisId)
                          << "ageMs=" << ageMs
                          << "bufferSize=" << connection.readBuffer.size()
                          << "state="
                          << (connection.socket != nullptr ? socketStateName(connection.socket->state())
                                                           : QStringLiteral("null"));

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
            qCDebug(logTcp) << "reconnect skip"
                            << axisName(axisId)
                            << "reason=not_configured";
            continue;
        }

        if (connection.socket != nullptr && (connection.socket->state() == QAbstractSocket::ConnectedState ||
                                             connection.socket->state() == QAbstractSocket::ConnectingState)) {
            qCDebug(logTcp) << "reconnect skip"
                            << axisName(axisId)
                            << "reason=active_socket"
                            << "state=" << socketStateName(connection.socket->state());
            continue;
        }

        const qint64 ageMs = currentMs - connection.lastReconnectAttemptMs;

        if (ageMs < reconnectIntervalMs) {
            qCDebug(logTcp) << "reconnect skip"
                            << axisName(axisId)
                            << "reason=too_early"
                            << "ageMs=" << ageMs
                            << "intervalMs=" << reconnectIntervalMs;
            continue;
        }

        qCInfo(logTcp) << "reconnect attempt"
                       << axisName(axisId)
                       << "lastAttemptAgeMs=" << ageMs;

        connectAxis(axisId);
    }
}

void QtTcpTelemetryClient::emitStatus(domain::AxisId axisId, domain::TelemetryConnectionStatus status,
                                      const std::string &message) {
    auto *connection = findAxis(axisId);
    if (connection != nullptr) {
        connection->status = status;
    }

    qCInfo(logTcp) << "status"
                   << axisName(axisId)
                   << static_cast<int>(status)
                   << QString::fromStdString(message);

    if (statusCallback) {
        statusCallback(axisId, status, message);
    }
}

void QtTcpTelemetryClient::emitError(domain::AxisId axisId, const std::string &message) {
    qCWarning(logTcp) << "error"
                      << axisName(axisId)
                      << QString::fromStdString(message);

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