#ifndef APPLYWINDDIRECTIONUSECASE_HPP
#define APPLYWINDDIRECTIONUSECASE_HPP

#include "../Session/SessionState.hpp"

namespace application::useCases {

class ApplyWindDirectionUseCase final {
  public:
    explicit ApplyWindDirectionUseCase(application::session::SessionState &state);

    bool execute(double directionDegrees);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // APPLYWINDDIRECTIONUSECASE_HPP
