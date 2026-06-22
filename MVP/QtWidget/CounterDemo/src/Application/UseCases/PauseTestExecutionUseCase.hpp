#ifndef PAUSETESTEXECUTIONUSECASE_HPP
#define PAUSETESTEXECUTIONUSECASE_HPP

#include "../Ports/ITelemetryClient.hpp"
#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Services/TelemetrySessionClock.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class PauseTestExecutionUseCase final {
  public:
    PauseTestExecutionUseCase(application::session::SessionState &state,
                              application::ports::ITestExecutionScheduler &testExecutionScheduler,
                              application::ports::ITelemetryClient &telemetryClient);
    PauseTestExecutionUseCase(application::session::SessionState &state,
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

#endif // PAUSETESTEXECUTIONUSECASE_HPP
