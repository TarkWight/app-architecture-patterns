#ifndef SETSTANDCONTROLMODEUSECASE_HPP
#define SETSTANDCONTROLMODEUSECASE_HPP

#include "../../Domain/StandControlMode.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class SetStandControlModeUseCase final {
  public:
    explicit SetStandControlModeUseCase(application::session::SessionState &state);

    void execute(domain::StandControlMode mode);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETSTANDCONTROLMODEUSECASE_HPP
