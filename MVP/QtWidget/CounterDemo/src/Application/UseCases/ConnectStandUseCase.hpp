#ifndef CONNECTSTANDUSECASE_HPP
#define CONNECTSTANDUSECASE_HPP

#include "../Ports/ITelemetryClient.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class ConnectStandUseCase final {
  public:
    ConnectStandUseCase(application::session::SessionState &state,
                        application::ports::ITelemetryClient &telemetryClient);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITelemetryClient &telemetryClient;
};

} // namespace application::useCases

#endif // CONNECTSTANDUSECASE_HPP
