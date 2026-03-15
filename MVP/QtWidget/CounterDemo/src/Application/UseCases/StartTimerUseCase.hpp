#ifndef STARTTIMERUSECASE_HPP
#define STARTTIMERUSECASE_HPP

#include "../Ports/ITimerService.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class StartTimerUseCase final {
  public:
    StartTimerUseCase(application::session::SessionState &state, application::ports::ITimerService &timerService);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITimerService &timerService;
};

} // namespace application::useCases

#endif // STARTTIMERUSECASE_HPP
