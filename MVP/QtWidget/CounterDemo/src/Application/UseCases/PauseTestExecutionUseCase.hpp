#ifndef PAUSETESTEXECUTIONUSECASE_HPP
#define PAUSETESTEXECUTIONUSECASE_HPP

#include "../Ports/ITestExecutionScheduler.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class PauseTestExecutionUseCase final {
  public:
    PauseTestExecutionUseCase(application::session::SessionState &state,
                              application::ports::ITestExecutionScheduler &testExecutionScheduler);

    void execute();

  private:
    application::session::SessionState &state;
    application::ports::ITestExecutionScheduler &testExecutionScheduler;
};

} // namespace application::useCases

#endif // PAUSETESTEXECUTIONUSECASE_HPP