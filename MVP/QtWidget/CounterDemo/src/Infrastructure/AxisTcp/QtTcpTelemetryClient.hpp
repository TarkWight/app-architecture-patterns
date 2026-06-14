#ifndef QTTCPTELEMETRYCLIENT_HPP
#define QTTCPTELEMETRYCLIENT_HPP

#include "../../Application/Ports/IAxisProtocolCodec.hpp"
#include "../../Application/Ports/ITelemetryClient.hpp"

#include <QByteArray>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

#include <memory>
#include <string>
#include <unordered_map>

namespace infrastructure::axisTcp {

class QtTcpTelemetryClient final : public QObject, public application::ports::ITelemetryClient {
    Q_OBJECT

  public:
    explicit QtTcpTelemetryClient(application::ports::IAxisProtocolCodec &codec, QObject *parent = nullptr);

    ~QtTcpTelemetryClient() override;

    void setTelemetryCallback(TelemetryCallback callback) override;
    void setStatusCallback(StatusCallback callback) override;
    void setErrorCallback(ErrorCallback callback) override;

    void configureAxis(domain::AxisId axisId, std::string host, int port) override;

    void connectAxis(domain::AxisId axisId) override;
    void disconnectAxis(domain::AxisId axisId) override;

    void connectAll() override;
    void disconnectAll() override;

    void startPolling(int intervalMs) override;
    void stopPolling() override;

    void setAxisCommand(domain::AxisId axisId, domain::AxisControlCommand command) override;
    void pollOnce(domain::AxisId axisId) override;

  private:
    struct AxisConnection {
        QString host{};
        int port{0};

        std::unique_ptr<QTcpSocket> socket{};

        domain::AxisControlCommand command{};
        domain::TelemetryConnectionStatus status{domain::TelemetryConnectionStatus::Disconnected};

        QByteArray readBuffer{};

        bool configured{false};
        bool requestPending{false};

        qint64 lastRequestMs{0};
        qint64 lastReconnectAttemptMs{0};
    };

    application::ports::IAxisProtocolCodec &codec;

    std::unordered_map<domain::AxisId, AxisConnection, domain::AxisIdHash> axes{};

    QTimer pollTimer;

    TelemetryCallback telemetryCallback{};
    StatusCallback statusCallback{};
    ErrorCallback errorCallback{};

    int pollingIntervalMs{1000};

    static constexpr int responseTimeoutMs{2000};
    static constexpr int reconnectIntervalMs{3000};
    static constexpr int expectedTelemetryResponseSize{30};

    AxisConnection *findAxis(domain::AxisId axisId);

    void createSocketIfNeeded(domain::AxisId axisId, AxisConnection &connection);

    void handlePollTimer();

    void handleConnected(domain::AxisId axisId);
    void handleDisconnected(domain::AxisId axisId);
    void handleSocketError(domain::AxisId axisId, QAbstractSocket::SocketError error);
    void handleReadyRead(domain::AxisId axisId);

    void processReadBuffer(domain::AxisId axisId, AxisConnection &connection);
    void handlePendingTimeouts();
    void reconnectDisconnectedAxes();

    void emitStatus(domain::AxisId axisId, domain::TelemetryConnectionStatus status, const std::string &message);

    void emitError(domain::AxisId axisId, const std::string &message);

    static qint64 nowMs();
    static double nowSeconds();
};

} // namespace infrastructure::axisTcp

#endif // QTTCPTELEMETRYCLIENT_HPP