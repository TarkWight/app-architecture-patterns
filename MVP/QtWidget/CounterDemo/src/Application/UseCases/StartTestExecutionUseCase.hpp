#ifndef STARTTESTEXECUTIONUSECASE_HPP
#define STARTTESTEXECUTIONUSECASE_HPP

#include "../Ports/ITelemetryClient.hpp"
#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class StartTestExecutionUseCase final {
  public:
    StartTestExecutionUseCase(application::session::SessionState &state,
                              application::ports::ITestExecutionScheduler &testExecutionScheduler,
                              application::ports::ITelemetryClient &telemetryClient);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITestExecutionScheduler &testExecutionScheduler;
    application::ports::ITelemetryClient &telemetryClient;

    void applyScenarioImpact(int elapsedSeconds);
    void sendAppliedImpact(const domain::WindProfile &profile);
};

} // namespace application::useCases

#endif // STARTTESTEXECUTIONUSECASE_HPP
