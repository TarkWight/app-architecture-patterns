#ifndef APPLYWINDDIRECTIONUSECASE_HPP
#define APPLYWINDDIRECTIONUSECASE_HPP

#include "../Session/SessionState.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::useCases {

class ApplyWindDirectionUseCase final {
  public:
    explicit ApplyWindDirectionUseCase(application::session::SessionState &state);

    bool execute(domain::WindDirection direction);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // APPLYWINDDIRECTIONUSECASE_HPP
