#ifndef SETSTANDIMPACTUSECASE_HPP
#define SETSTANDIMPACTUSECASE_HPP

#include "../../Domain/WindImpact.hpp"
#include "../../Domain/Time.hpp"
#include "../Ports/ITelemetryClient.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class SetStandImpactUseCase final {
  public:
    SetStandImpactUseCase(application::session::SessionState &state,
                          application::ports::ITelemetryClient &telemetryClient);

    void setTarget(domain::WindImpact profile);
    void setApplied(domain::WindImpact profile);

  private:
    application::session::SessionState &state;
    application::ports::ITelemetryClient &telemetryClient;

    void sendAppliedImpact(const domain::WindImpact &profile, domain::ElapsedSeconds elapsed);
};

} // namespace application::useCases

#endif // SETSTANDIMPACTUSECASE_HPP
