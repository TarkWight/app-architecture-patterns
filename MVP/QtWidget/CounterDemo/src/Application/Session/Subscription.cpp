#include "Subscription.hpp"

namespace application::session {

Subscription::Subscription() = default;

Subscription::Subscription(std::function<void()> unsubscribe) : unsubscribe(std::move(unsubscribe)) {
}

Subscription::Subscription(Subscription &&other) noexcept : unsubscribe(std::move(other.unsubscribe)) {
    other.unsubscribe = {};
}

Subscription &application::session::Subscription::operator=(Subscription &&other) noexcept {
    if (this != &other) {
        reset();
        unsubscribe = std::move(other.unsubscribe);
        other.unsubscribe = {};
    }
    return *this;
}

Subscription::~Subscription() {
    reset();
}

void Subscription::reset() {
    if (unsubscribe) {
        unsubscribe();
        unsubscribe = {};
    }
}

} // namespace application::session
