#ifndef RESUMETESTEXECUTIONUSECASE_HPP
#define RESUMETESTEXECUTIONUSECASE_HPP

#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class ResumeTestExecutionUseCase final {
  public:
    ResumeTestExecutionUseCase(application::session::SessionState &state,
                               application::ports::ITestExecutionScheduler &testExecutionScheduler);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITestExecutionScheduler &testExecutionScheduler;
};

} // namespace application::useCases

#endif // RESUMETESTEXECUTIONUSECASE_HPP