#ifndef TELEMETRYWINDOW_HPP
#define TELEMETRYWINDOW_HPP

#include "AxisTelemetrySample.hpp"

#include <algorithm>

namespace domain {

class TelemetryWindowEnd final {
  public:
    static TelemetryWindowEnd fromSeconds(double rawSeconds) {
        return TelemetryWindowEnd{std::max(0.0, rawSeconds)};
    }

    static TelemetryWindowEnd fromTail(const AxisTelemetrySample &first, const AxisTelemetrySample &last) {
        return fromSeconds(last.timestampSeconds - first.timestampSeconds);
    }

    [[nodiscard]] double seconds() const {
        return rawSeconds;
    }

  private:
    explicit TelemetryWindowEnd(double seconds) : rawSeconds(seconds) {
    }

    double rawSeconds{0.0};
};

} // namespace domain

#endif // TELEMETRYWINDOW_HPP
