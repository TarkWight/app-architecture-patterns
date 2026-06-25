#ifndef APPLIEDSTANDIMPACTSENDER_HPP
#define APPLIEDSTANDIMPACTSENDER_HPP

#include "../Ports/ITelemetryClient.hpp"

#include "../../Domain/TestProtocol.hpp"
#include "../../Domain/Time.hpp"
#include "../../Domain/WindImpact.hpp"

namespace application::services {

class AppliedStandImpactSender final {
  public:
    explicit AppliedStandImpactSender(application::ports::ITelemetryClient &telemetryClient);

    void send(const domain::WindImpact &impact, domain::ElapsedSeconds elapsed, const domain::TestProtocol &protocol,
              bool useAngleOfAttackModel) const;

  private:
    application::ports::ITelemetryClient &telemetryClient;
};

} // namespace application::services

#endif // APPLIEDSTANDIMPACTSENDER_HPP
