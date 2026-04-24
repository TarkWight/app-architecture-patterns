#ifndef ITESTEXECUTIONSCHEDULER_HPP
#define ITESTEXECUTIONSCHEDULER_HPP

#include <functional>

namespace application::ports {

class ITestExecutionScheduler {
  public:
    using TickCallback = std::function<void(int elapsedSeconds)>;

    virtual ~ITestExecutionScheduler() = default;

    virtual void start(int initialElapsedSeconds, TickCallback onTick) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;

    virtual bool isRunning() const = 0;
    virtual bool isPaused() const = 0;
};

} // namespace application::ports

#endif // ITESTEXECUTIONSCHEDULER_HPP