#include "QtTestExecutionScheduler.hpp"

#include <utility>

namespace infrastructure {

QtTestExecutionScheduler::QtTestExecutionScheduler(QObject *parent) : QtTestExecutionScheduler(1000, parent) {
}

QtTestExecutionScheduler::QtTestExecutionScheduler(int intervalMs, QObject *parent)
    : QObject(parent), intervalMs(intervalMs > 0 ? intervalMs : 1000) {
}

void QtTestExecutionScheduler::start(int initialElapsedSeconds, TickCallback onTick) {
    callback = std::move(onTick);
    elapsedSeconds = initialElapsedSeconds;
    state = State::Running;
    ++generation;

    scheduleNextTick();
}

void QtTestExecutionScheduler::pause() {
    if (state != State::Running) {
        return;
    }

    ++generation;
    state = State::Paused;
}

void QtTestExecutionScheduler::resume() {
    if (state != State::Paused) {
        return;
    }

    state = State::Running;
    ++generation;
    scheduleNextTick();
}

void QtTestExecutionScheduler::stop() {
    ++generation;
    callback = {};
    elapsedSeconds = 0;
    state = State::Idle;
}

bool QtTestExecutionScheduler::isRunning() const {
    return state == State::Running;
}

bool QtTestExecutionScheduler::isPaused() const {
    return state == State::Paused;
}

void QtTestExecutionScheduler::scheduleNextTick() {
    const auto expectedGeneration = generation;
    QTimer::singleShot(intervalMs, this, [this, expectedGeneration]() { handleTimeout(expectedGeneration); });
}

void QtTestExecutionScheduler::handleTimeout(std::uint64_t expectedGeneration) {
    if (state != State::Running || expectedGeneration != generation) {
        return;
    }

    ++elapsedSeconds;

    if (callback) {
        callback(elapsedSeconds);
    }

    if (state == State::Running && expectedGeneration == generation) {
        scheduleNextTick();
    }
}

} // namespace infrastructure
