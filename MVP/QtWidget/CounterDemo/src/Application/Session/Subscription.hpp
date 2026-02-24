#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <functional>

namespace application::session {

class Subscription {
  public:
    Subscription();
    explicit Subscription(std::function<void()> unsubscribe);

    Subscription(const Subscription &) = delete;
    Subscription &operator=(const Subscription &) = delete;

    Subscription(Subscription &&other) noexcept;

    Subscription &operator=(Subscription &&other) noexcept;

    ~Subscription();

    void reset();

  private:
    std::function<void()> unsubscribe{};
};

} // namespace application::session

#endif // SUBSCRIPTION_H
