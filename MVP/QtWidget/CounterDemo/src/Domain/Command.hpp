#ifndef COMMAND_HPP
#define COMMAND_HPP

namespace domain {

enum class CounterCommandKind {
    Increment,
    Decrement,
    Reset,
};

struct CounterCommand final {
    CounterCommandKind kind{};
    int delta{0};
};

} // namespace domain

#endif // COMMAND_HPP
