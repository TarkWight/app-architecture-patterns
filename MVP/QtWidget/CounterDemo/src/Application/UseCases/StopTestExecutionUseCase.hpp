#ifndef STOPTESTEXECUTIONUSECASE_HPP
#define STOPTESTEXECUTIONUSECASE_HPP

#include "../Ports/ITelemetryClient.hpp"
#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Services/TelemetrySessionClock.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class StopTestExecutionUseCase final {
  public:
    StopTestExecutionUseCase(application::session::SessionState &state,
                             application::ports::ITestExecutionScheduler &testExecutionScheduler,
                             application::ports::ITelemetryClient &telemetryClient);
    StopTestExecutionUseCase(application::session::SessionState &state,
                             application::ports::ITestExecutionScheduler &testExecutionScheduler,
                             application::ports::ITelemetryClient &telemetryClient,
                             application::services::TelemetrySessionClock &telemetrySessionClock);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITestExecutionScheduler &testExecutionScheduler;
    application::ports::ITelemetryClient &telemetryClient;
    application::services::TelemetrySessionClock *telemetrySessionClock{nullptr};
};

} // namespace application::useCases

#endif // STOPTESTEXECUTIONUSECASE_HPP
