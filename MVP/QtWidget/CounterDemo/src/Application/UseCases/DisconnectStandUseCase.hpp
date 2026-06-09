#ifndef DISCONNECTSTANDUSECASE_HPP
#define DISCONNECTSTANDUSECASE_HPP

#include "../Ports/ITelemetryClient.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class DisconnectStandUseCase final {
  public:
    DisconnectStandUseCase(application::session::SessionState &state,
                           application::ports::ITelemetryClient &telemetryClient);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITelemetryClient &telemetryClient;
};

} // namespace application::useCases

#endif // DISCONNECTSTANDUSECASE_HPP
