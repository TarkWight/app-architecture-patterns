#ifndef CONTROLTRACE_HPP
#define CONTROLTRACE_HPP

#include "Time.hpp"
#include "WindImpact.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace domain {

class ControlTraceTime final {
  public:
    static ControlTraceTime fromSeconds(double rawSeconds) {
        return ControlTraceTime{std::max(0.0, rawSeconds)};
    }

    static ControlTraceTime fromElapsed(ElapsedSeconds elapsed) {
        return fromSeconds(static_cast<double>(elapsed.value()));
    }

    [[nodiscard]] double seconds() const {
        return rawSeconds;
    }

    [[nodiscard]] double minutes() const {
        return rawSeconds / 60.0;
    }

    [[nodiscard]] bool operator<=(const ControlTraceTime &other) const {
        return rawSeconds <= other.rawSeconds;
    }

  private:
    explicit ControlTraceTime(double seconds) : rawSeconds(seconds) {
    }

    double rawSeconds{0.0};
};

struct ControlTraceSample {
    ControlTraceTime time{ControlTraceTime::fromSeconds(0.0)};
    WindImpact targetValue{};
    WindImpact safeCommandValue{};

    [[nodiscard]] static ControlTraceSample manualCommand(ElapsedSeconds elapsed, WindImpact target,
                                                          WindImpact safeCommand) {
        return ControlTraceSample{.time = ControlTraceTime::fromElapsed(elapsed),
                                  .targetValue = std::move(target),
                                  .safeCommandValue = std::move(safeCommand)};
    }
};

class ControlTrace final {
  public:
    static constexpr double minimumStepSeconds = 0.1;
    static constexpr std::size_t maxSamples = 50'000;

    void clear() {
        sampleList.clear();
    }

    void append(ControlTraceSample sample) {
        if (!sampleList.empty() && sample.time <= sampleList.back().time) {
            sample.time = ControlTraceTime::fromSeconds(sampleList.back().time.seconds() + minimumStepSeconds);
        }

        sampleList.push_back(std::move(sample));
        trimToMaxSamples();
    }

    [[nodiscard]] bool empty() const {
        return sampleList.empty();
    }

    [[nodiscard]] std::size_t size() const {
        return sampleList.size();
    }

    [[nodiscard]] const ControlTraceSample &front() const {
        return sampleList.front();
    }

    [[nodiscard]] const ControlTraceSample &back() const {
        return sampleList.back();
    }

    [[nodiscard]] const ControlTraceSample &at(std::size_t index) const {
        return sampleList.at(index);
    }

    [[nodiscard]] const std::vector<ControlTraceSample> &samples() const {
        return sampleList;
    }

  private:
    std::vector<ControlTraceSample> sampleList{};

    void trimToMaxSamples() {
        if (sampleList.size() <= maxSamples) {
            return;
        }

        const auto eraseCount = sampleList.size() - maxSamples;
        sampleList.erase(sampleList.begin(),
                         sampleList.begin() +
                             static_cast<std::vector<ControlTraceSample>::difference_type>(eraseCount));
    }
};

} // namespace domain

#endif // CONTROLTRACE_HPP
