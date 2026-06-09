#ifndef ITELEMETRYCLIENT_HPP
#define ITELEMETRYCLIENT_HPP

#include "../../Domain/AxisControlCommand.hpp"
#include "../../Domain/AxisId.hpp"
#include "../../Domain/AxisTelemetrySample.hpp"
#include "../../Domain/TelemetryConnectionStatus.hpp"

#include <functional>
#include <string>

namespace application::ports {

class ITelemetryClient {
  public:
    using TelemetryCallback = std::function<void(const domain::AxisTelemetrySample &)>;
    using StatusCallback = std::function<void(domain::AxisId, domain::TelemetryConnectionStatus, const std::string &)>;
    using ErrorCallback = std::function<void(domain::AxisId, const std::string &)>;

    virtual ~ITelemetryClient() = default;

    virtual void setTelemetryCallback(TelemetryCallback callback) = 0;
    virtual void setStatusCallback(StatusCallback callback) = 0;
    virtual void setErrorCallback(ErrorCallback callback) = 0;

    virtual void configureAxis(domain::AxisId axisId, std::string host, int port) = 0;

    virtual void connectAxis(domain::AxisId axisId) = 0;
    virtual void disconnectAxis(domain::AxisId axisId) = 0;

    virtual void connectAll() = 0;
    virtual void disconnectAll() = 0;

    virtual void startPolling(int intervalMs) = 0;
    virtual void stopPolling() = 0;

    virtual void setAxisCommand(domain::AxisId axisId, domain::AxisControlCommand command) = 0;
    virtual void pollOnce(domain::AxisId axisId) = 0;
};

} // namespace application::ports

#endif // ITELEMETRYCLIENT_HPP