#ifndef COUNTER_HPP
#define COUNTER_HPP

#include "CounterId.hpp"
#include "Command.hpp"

namespace domain {

class Counter final {
  public:
    explicit Counter(CounterId id);

    CounterId getId() const;
    int getValue() const;
    void apply(const CounterCommand &command);

  private:
    CounterId id;
    int value{0};
};

} // namespace domain

#endif // COUNTER_HPP
