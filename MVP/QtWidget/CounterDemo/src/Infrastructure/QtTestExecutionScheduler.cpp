#include "QtTestExecutionScheduler.hpp"

#include <utility>

namespace infrastructure {

QtTestExecutionScheduler::QtTestExecutionScheduler(QObject *parent) : QObject(parent) {
    timer.setInterval(1000);
    QObject::connect(&timer, &QTimer::timeout, this, &QtTestExecutionScheduler::handleTimeout);
}

void QtTestExecutionScheduler::start(int initialElapsedSeconds, TickCallback onTick) {
    callback = std::move(onTick);
    elapsedSeconds = initialElapsedSeconds;
    state = State::Running;

    if (callback) {
        callback(elapsedSeconds);
    }

    timer.start();
}

void QtTestExecutionScheduler::pause() {
    if (state != State::Running) {
        return;
    }

    timer.stop();
    state = State::Paused;
}

void QtTestExecutionScheduler::resume() {
    if (state != State::Paused) {
        return;
    }

    state = State::Running;
    timer.start();
}

void QtTestExecutionScheduler::stop() {
    timer.stop();
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

void QtTestExecutionScheduler::handleTimeout() {
    if (state != State::Running) {
        return;
    }

    ++elapsedSeconds;

    if (callback) {
        callback(elapsedSeconds);
    }
}

} // namespace infrastructure