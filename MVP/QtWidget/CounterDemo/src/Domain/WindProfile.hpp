#ifndef WINDPROFILE_HPP
#define WINDPROFILE_HPP

#include "Expression.hpp"

#include <algorithm>
#include <cmath>

namespace domain {

constexpr double minOperationalBeaufort = 0.0;
constexpr double maxOperationalBeaufort = 7.0;

struct WindProfile {
    double beaufort{0.0};
    double direction{0.0};
    double angleOfAttack{0.0};
    Expression formula{};
};

inline WindProfile sanitize(WindProfile profile) {
    profile.beaufort = std::clamp(profile.beaufort, minOperationalBeaufort, maxOperationalBeaufort);

    profile.direction = std::fmod(profile.direction, 360.0);
    if (profile.direction < 0.0) {
        profile.direction += 360.0;
    }

    profile.angleOfAttack = std::clamp(profile.angleOfAttack, -90.0, 90.0);

    return profile;
}

} // namespace domain

#endif // WINDPROFILE_HPP
