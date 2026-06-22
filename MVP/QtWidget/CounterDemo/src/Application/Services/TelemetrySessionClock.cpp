#include "TelemetrySessionClock.hpp"

#include <algorithm>

namespace application::services {

void TelemetrySessionClock::reset() {
    initialized = false;
    paused = false;
    resuming = false;
    firstRawTimestampSeconds = 0.0;
    lastRawTimestampSeconds = 0.0;
    lastLogicalTimestampSeconds = 0.0;
    accumulatedPausedSeconds = 0.0;
    expectedSampleIntervalSeconds = 1.0;
}

void TelemetrySessionClock::pause() {
    paused = true;
    resuming = false;
}

void TelemetrySessionClock::resume() {
    if (!paused) {
        return;
    }

    paused = false;
    resuming = initialized;
}

std::optional<domain::AxisTelemetrySample> TelemetrySessionClock::map(domain::AxisTelemetrySample sample) {
    if (!sample.valid || paused) {
        return std::nullopt;
    }

    if (!initialized) {
        initialized = true;
        firstRawTimestampSeconds = sample.timestampSeconds;
        lastRawTimestampSeconds = sample.timestampSeconds;
        lastLogicalTimestampSeconds = 0.0;
        sample.timestampSeconds = 0.0;
        return sample;
    }

    const double rawDelta = std::max(0.0, sample.timestampSeconds - lastRawTimestampSeconds);

    if (resuming) {
        accumulatedPausedSeconds += std::max(0.0, rawDelta - expectedSampleIntervalSeconds);
        resuming = false;
    } else if (rawDelta > 0.0) {
        expectedSampleIntervalSeconds = std::min(rawDelta, 1.0);
    }

    const double logicalTimestamp = std::max(
        lastLogicalTimestampSeconds, sample.timestampSeconds - firstRawTimestampSeconds - accumulatedPausedSeconds);

    lastRawTimestampSeconds = sample.timestampSeconds;
    lastLogicalTimestampSeconds = logicalTimestamp;
    sample.timestampSeconds = logicalTimestamp;
    return sample;
}

} // namespace application::services
