#ifndef APPLYANGLEOFATTACKUSECASE_HPP
#define APPLYANGLEOFATTACKUSECASE_HPP

#include "../Session/SessionState.hpp"

namespace application::useCases {

class ApplyAngleOfAttackUseCase final {
  public:
    explicit ApplyAngleOfAttackUseCase(application::session::SessionState &state);

    bool execute(double angleOfAttackDegrees);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // APPLYANGLEOFATTACKUSECASE_HPP
