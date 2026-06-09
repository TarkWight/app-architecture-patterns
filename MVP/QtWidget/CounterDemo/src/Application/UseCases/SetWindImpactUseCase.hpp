#ifndef SETWINDIMPACTUSECASE_HPP
#define SETWINDIMPACTUSECASE_HPP

#include "../Session/SessionState.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::useCases {

class SetWindImpactUseCase final {
  public:
    explicit SetWindImpactUseCase(application::session::SessionState &state);

    void execute(domain::WindImpact profile);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETWINDIMPACTUSECASE_HPP
