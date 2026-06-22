#ifndef TELEMETRYPOLLINTERVAL_HPP
#define TELEMETRYPOLLINTERVAL_HPP

#include <algorithm>

namespace domain {

constexpr int minTelemetryPollIntervalMs = 20;
constexpr int maxTelemetryPollIntervalMs = 60'000;

class TelemetryPollInterval final {
  public:
    static TelemetryPollInterval fromMilliseconds(int rawValue) {
        return TelemetryPollInterval{std::clamp(rawValue, minTelemetryPollIntervalMs, maxTelemetryPollIntervalMs)};
    }

    [[nodiscard]] int milliseconds() const {
        return rawValue;
    }

  private:
    explicit TelemetryPollInterval(int value) : rawValue(value) {
    }

    int rawValue{1000};
};

} // namespace domain

#endif // TELEMETRYPOLLINTERVAL_HPP
