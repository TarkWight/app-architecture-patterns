#ifndef RESUMETESTEXECUTIONUSECASE_HPP
#define RESUMETESTEXECUTIONUSECASE_HPP

#include "../Ports/ITelemetryClient.hpp"
#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Services/TelemetrySessionClock.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class ResumeTestExecutionUseCase final {
  public:
    ResumeTestExecutionUseCase(application::session::SessionState &state,
                               application::ports::ITestExecutionScheduler &testExecutionScheduler,
                               application::ports::ITelemetryClient &telemetryClient);
    ResumeTestExecutionUseCase(application::session::SessionState &state,
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

#endif // RESUMETESTEXECUTIONUSECASE_HPP
