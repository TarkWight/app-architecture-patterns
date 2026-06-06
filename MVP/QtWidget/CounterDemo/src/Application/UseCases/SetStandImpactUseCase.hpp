#ifndef SETSTANDIMPACTUSECASE_HPP
#define SETSTANDIMPACTUSECASE_HPP

#include "../../Domain/WindProfile.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class SetStandImpactUseCase final {
  public:
    explicit SetStandImpactUseCase(application::session::SessionState &state);

    void setTarget(domain::WindProfile profile);
    void setApplied(domain::WindProfile profile);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETSTANDIMPACTUSECASE_HPP
