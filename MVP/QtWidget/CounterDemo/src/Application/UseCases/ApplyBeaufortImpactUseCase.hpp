#ifndef APPLYBEAUFORTIMPACTUSECASE_HPP
#define APPLYBEAUFORTIMPACTUSECASE_HPP

#include "../Session/SessionState.hpp"

namespace application::useCases {

class ApplyBeaufortImpactUseCase final {
  public:
    explicit ApplyBeaufortImpactUseCase(application::session::SessionState &state);

    bool execute(double beaufort);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // APPLYBEAUFORTIMPACTUSECASE_HPP
