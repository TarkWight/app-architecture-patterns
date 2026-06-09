#pragma once

#include <algorithm>

namespace domain {

constexpr int minDurationMinutes = 1;
constexpr int maxDurationMinutes = 24 * 60;

class DurationMinutes final {
  public:
    static DurationMinutes required(int rawValue) {
        return DurationMinutes{std::clamp(rawValue, minDurationMinutes, maxDurationMinutes)};
    }

    static DurationMinutes optional(int rawValue) {
        return DurationMinutes{std::clamp(rawValue, 0, maxDurationMinutes)};
    }

    [[nodiscard]] int value() const {
        return rawValue;
    }

  private:
    explicit DurationMinutes(int value) : rawValue(value) {
    }

    int rawValue{minDurationMinutes};
};

class ElapsedSeconds final {
  public:
    static ElapsedSeconds from(int rawValue) {
        return ElapsedSeconds{std::max(0, rawValue)};
    }

    [[nodiscard]] int value() const {
        return rawValue;
    }

  private:
    explicit ElapsedSeconds(int value) : rawValue(value) {
    }

    int rawValue{0};
};

class RemainingSeconds final {
  public:
    static RemainingSeconds from(int rawValue) {
        return RemainingSeconds{std::max(0, rawValue)};
    }

    [[nodiscard]] int value() const {
        return rawValue;
    }

  private:
    explicit RemainingSeconds(int value) : rawValue(value) {
    }

    int rawValue{0};
};

} // namespace domain
