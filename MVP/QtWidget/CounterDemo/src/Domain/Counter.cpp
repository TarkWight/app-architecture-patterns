#include "Counter.hpp"

namespace domain {

Counter::Counter(CounterId idd) : id(idd) {
}

CounterId Counter::getId() const {
    return id;
}
int Counter::getValue() const {
    return value;
}

void Counter::apply(const CounterCommand &command) {
    switch (command.kind) {
    case CounterCommandKind::Increment:
        value += command.delta;
        break;
    case CounterCommandKind::Decrement:
        value -= command.delta;
        break;
    case CounterCommandKind::Reset:
        value = 0;
        break;
    }
}

} // namespace domain
