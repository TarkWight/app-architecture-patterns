#ifndef SETTELEMETRYAXISVISIBLEUSECASE_HPP
#define SETTELEMETRYAXISVISIBLEUSECASE_HPP

#include "../../Domain/AxisId.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class SetTelemetryAxisVisibleUseCase final {
  public:
    explicit SetTelemetryAxisVisibleUseCase(application::session::SessionState &state);

    void execute(domain::AxisId axisId, bool visible);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETTELEMETRYAXISVISIBLEUSECASE_HPP
