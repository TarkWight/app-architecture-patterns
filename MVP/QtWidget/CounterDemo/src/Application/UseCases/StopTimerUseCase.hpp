#ifndef STOPTIMERUSECASE_HPP
#define STOPTIMERUSECASE_HPP

#include "../Ports/ITimerService.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class StopTimerUseCase final {
  public:
    StopTimerUseCase(application::session::SessionState &state, application::ports::ITimerService &timerService);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITimerService &timerService;
};

} // namespace application::useCases

#endif // STOPTIMERUSECASE_HPP
