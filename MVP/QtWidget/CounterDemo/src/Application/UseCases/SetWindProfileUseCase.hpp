#ifndef SETWINDPROFILEUSECASE_HPP
#define SETWINDPROFILEUSECASE_HPP

#include "../Session/SessionState.hpp"
#include "../../Domain/WindProfile.hpp"

namespace application::useCases {

class SetWindProfileUseCase final {
  public:
    explicit SetWindProfileUseCase(application::session::SessionState &state);

    void execute(domain::WindProfile profile);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETWINDPROFILEUSECASE_HPP
