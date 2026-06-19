#ifndef TELEMETRYSESSIONCLOCK_HPP
#define TELEMETRYSESSIONCLOCK_HPP

#include "../../Domain/AxisTelemetrySample.hpp"

#include <optional>

namespace application::services {

class TelemetrySessionClock final {
  public:
    void reset();
    void pause();
    void resume();

    [[nodiscard]] std::optional<domain::AxisTelemetrySample> map(domain::AxisTelemetrySample sample);

  private:
    bool initialized{false};
    bool paused{false};
    bool resuming{false};

    double firstRawTimestampSeconds{0.0};
    double lastRawTimestampSeconds{0.0};
    double lastLogicalTimestampSeconds{0.0};
    double accumulatedPausedSeconds{0.0};
    double expectedSampleIntervalSeconds{1.0};
};

} // namespace application::services

#endif // TELEMETRYSESSIONCLOCK_HPP
