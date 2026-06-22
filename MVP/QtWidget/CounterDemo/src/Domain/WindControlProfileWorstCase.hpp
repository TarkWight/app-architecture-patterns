#ifndef WINDCONTROLPROFILEWORSTCASE_HPP
#define WINDCONTROLPROFILEWORSTCASE_HPP

#include "WindControlProfile.hpp"
#include "WindImpact.hpp"

namespace domain {

struct WorstCaseWindImpact {
    WindImpact impact{};
    bool usedProfile{false};
};

class WindControlProfileWorstCase final {
  public:
    [[nodiscard]] static WorstCaseWindImpact from(const WindControlProfile &profile, const WindImpact &fallbackImpact) {
        auto worstImpact = fallbackImpact;
        bool usedProfile = false;
        double maxBeaufort = fallbackImpact.beaufort.value();

        for (const auto &sample : profile.samples) {
            if (sample.beaufort.value() > maxBeaufort) {
                worstImpact = worstImpact.withBeaufort(sample.beaufort);
                usedProfile = true;
                maxBeaufort = sample.beaufort.value();
            }
        }

        return WorstCaseWindImpact{.impact = worstImpact, .usedProfile = usedProfile};
    }
};

} // namespace domain

#endif // WINDCONTROLPROFILEWORSTCASE_HPP
