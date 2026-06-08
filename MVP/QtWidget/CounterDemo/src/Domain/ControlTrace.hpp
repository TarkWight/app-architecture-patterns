#ifndef CONTROLTRACE_HPP
#define CONTROLTRACE_HPP

#include "WindProfile.hpp"

namespace domain {

struct ControlTraceSample {
    double timeSeconds{0.0};
    WindProfile targetValue{};
    WindProfile safeCommandValue{};
};

} // namespace domain

#endif // CONTROLTRACE_HPP
