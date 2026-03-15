#include "QtTimerService.hpp"

namespace infrastructure {

QtTimerService::QtTimerService(QObject *parent) : QObject(parent) {
    timer.setInterval(1000);

    QObject::connect(&timer, &QTimer::timeout, this, &QtTimerService::handleTimeout);
}

void QtTimerService::start(TickCallback onTick) {
    callback = std::move(onTick);
    elapsedSeconds = 0;

    if (callback) {
        callback(elapsedSeconds);
    }

    timer.start();
}

void QtTimerService::stop() {
    timer.stop();
    callback = {};
}

void QtTimerService::handleTimeout() {
    ++elapsedSeconds;

    if (callback) {
        callback(elapsedSeconds);
    }
}

} // namespace infrastructure
