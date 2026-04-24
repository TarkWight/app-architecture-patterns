#ifndef STARTTESTEXECUTIONUSECASE_HPP
#define STARTTESTEXECUTIONUSECASE_HPP

#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class StartTestExecutionUseCase final {
  public:
    StartTestExecutionUseCase(application::session::SessionState &state,
                              application::ports::ITestExecutionScheduler &testExecutionScheduler);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITestExecutionScheduler &testExecutionScheduler;
};

} // namespace application::useCases

#endif // STARTTESTEXECUTIONUSECASE_HPP