#ifndef WINDPROFILE_HPP
#define WINDPROFILE_HPP

#include "Expression.hpp"

namespace domain {

struct WindProfile {
    double beaufort{0.0};
    double direction{0.0};
    double angleOfAttack{0.0};
    Expression formula{};
};

} // namespace domain

#endif // WINDPROFILE_HPP
