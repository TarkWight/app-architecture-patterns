#ifndef SETTELEMETRYAXISCOLORUSECASE_HPP
#define SETTELEMETRYAXISCOLORUSECASE_HPP

#include "../../Domain/AxisId.hpp"
#include "../../Domain/Plot.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class SetTelemetryAxisColorUseCase final {
  public:
    explicit SetTelemetryAxisColorUseCase(application::session::SessionState &state);

    void execute(domain::AxisId axisId, domain::RgbColor color);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETTELEMETRYAXISCOLORUSECASE_HPP
