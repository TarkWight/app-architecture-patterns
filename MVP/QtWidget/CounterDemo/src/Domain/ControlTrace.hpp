#ifndef CONTROLTRACE_HPP
#define CONTROLTRACE_HPP

#include "Time.hpp"
#include "WindImpact.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace domain {

struct ControlTraceSample {
    double timeSeconds{0.0};
    WindImpact targetValue{};
    WindImpact safeCommandValue{};

    [[nodiscard]] static ControlTraceSample manualCommand(ElapsedSeconds elapsed, WindImpact target,
                                                          WindImpact safeCommand) {
        return ControlTraceSample{.timeSeconds = static_cast<double>(elapsed.value()),
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
        if (!sampleList.empty() && sample.timeSeconds <= sampleList.back().timeSeconds) {
            sample.timeSeconds = sampleList.back().timeSeconds + minimumStepSeconds;
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
