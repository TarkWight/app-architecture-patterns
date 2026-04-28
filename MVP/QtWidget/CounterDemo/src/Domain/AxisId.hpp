#ifndef AXISID_HPP
#define AXISID_HPP

#include "Id.hpp"

#include <cstdint>

namespace domain {

struct AxisIdTag final {};

using AxisId = Id<AxisIdTag, std::uint8_t>;

inline constexpr AxisId Axis0{0};
inline constexpr AxisId Axis1{1};

} // namespace domain

#endif // AXISID_HPP