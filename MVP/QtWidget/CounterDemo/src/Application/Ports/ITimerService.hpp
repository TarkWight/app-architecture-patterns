#ifndef ITIMERSERVICE_HPP
#define ITIMERSERVICE_HPP

#include <functional>

namespace application::ports {

class ITimerService {
  public:
    using TickCallback = std::function<void(int elapsedSeconds)>;

    virtual ~ITimerService() = default;

    virtual void start(TickCallback onTick) = 0;
    virtual void stop() = 0;
};

} // namespace application::ports

#endif // ITIMERSERVICE_HPP
