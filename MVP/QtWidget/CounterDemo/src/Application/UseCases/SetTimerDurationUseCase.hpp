#ifndef SETTIMERDURATIONUSECASE_HPP
#define SETTIMERDURATIONUSECASE_HPP

#include "../Session/SessionState.hpp"

namespace application::useCases {

class SetTimerDurationUseCase final {
  public:
    explicit SetTimerDurationUseCase(application::session::SessionState &state);

    void execute(int minutes);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETTIMERDURATIONUSECASE_HPP
