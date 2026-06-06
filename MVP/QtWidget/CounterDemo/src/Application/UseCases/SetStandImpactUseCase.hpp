#ifndef SETSTANDIMPACTUSECASE_HPP
#define SETSTANDIMPACTUSECASE_HPP

#include "../../Domain/WindProfile.hpp"
#include "../Ports/ITelemetryClient.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class SetStandImpactUseCase final {
  public:
    SetStandImpactUseCase(application::session::SessionState &state,
                          application::ports::ITelemetryClient &telemetryClient);

    void setTarget(domain::WindProfile profile);
    void setApplied(domain::WindProfile profile);

  private:
    application::session::SessionState &state;
    application::ports::ITelemetryClient &telemetryClient;

    void sendAppliedImpact(const domain::WindProfile &profile);
};

} // namespace application::useCases

#endif // SETSTANDIMPACTUSECASE_HPP
