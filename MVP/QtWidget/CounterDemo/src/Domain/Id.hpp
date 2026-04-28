#ifndef ID_HPP
#define ID_HPP

#include <cstdint>
#include <compare> // for <=>

namespace domain {

template <typename Tag, typename Rep = std::uint32_t>
class Id final {
public:
    using ValueType = Rep;

    constexpr Id() = default;

    explicit constexpr Id(Rep value) : value(value) {
    }

    [[nodiscard]] constexpr Rep getValue() const {
        return value;
    }

    friend constexpr auto operator<=>(const Id &, const Id &) = default;

private:
    Rep value{};
};

} // namespace domain

#endif // ID_HPP