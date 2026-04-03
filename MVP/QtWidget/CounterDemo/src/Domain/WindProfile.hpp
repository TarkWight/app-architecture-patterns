#ifndef WINDPROFILE_HPP
#define WINDPROFILE_HPP

#include "FormulaExpression.hpp"

namespace domain {

struct WindProfile {
    double beaufort{0.0};
    double direction{0.0};
    double angleOfAttack{0.0};
    FormulaExpression formula{};
};

} // namespace domain

#endif // WINDPROFILE_HPP
