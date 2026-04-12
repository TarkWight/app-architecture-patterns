#ifndef STOPTESTEXECUTIONUSECASE_HPP
#define STOPTESTEXECUTIONUSECASE_HPP

#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class StopTestExecutionUseCase final {
  public:
    StopTestExecutionUseCase(application::session::SessionState &state,
                             application::ports::ITestExecutionScheduler &testExecutionScheduler);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITestExecutionScheduler &testExecutionScheduler;
};

} // namespace application::useCases

#endif // STOPTESTEXECUTIONUSECASE_HPP