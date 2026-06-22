#ifndef SETTELEMETRYWINDOWUSECASE_HPP
#define SETTELEMETRYWINDOWUSECASE_HPP

#include "../Session/SessionState.hpp"

#include "../../Domain/TelemetryWindow.hpp"

namespace application::useCases {

class SetTelemetryWindowUseCase final {
  public:
    explicit SetTelemetryWindowUseCase(application::session::SessionState &state);

    void execute(domain::TelemetryWindowEnd windowEndSeconds);
    void followTail();

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETTELEMETRYWINDOWUSECASE_HPP
