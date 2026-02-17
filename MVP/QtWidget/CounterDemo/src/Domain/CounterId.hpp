#ifndef COUNTERID_HPP
#define COUNTERID_HPP

#include <cstdint>

namespace domain {

class CounterId final {
  public:
    using Rep = std::uint8_t;

    constexpr explicit CounterId(Rep value) : value(value) {
    }

    constexpr Rep getValue() {
        return value;
    }

    friend constexpr bool operator==(CounterId, CounterId) = default;

  private:
    Rep value;
};

} // namespace domain

#endif // COUNTERID_HPP
