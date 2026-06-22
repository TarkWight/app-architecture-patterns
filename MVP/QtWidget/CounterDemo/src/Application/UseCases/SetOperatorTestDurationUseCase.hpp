#ifndef SETTIMERDURATIONUSECASE_HPP
#define SETTIMERDURATIONUSECASE_HPP

#include "../Session/SessionState.hpp"

namespace application::useCases {

class SetOperatorTestDurationUseCase final {
  public:
    explicit SetOperatorTestDurationUseCase(application::session::SessionState &state);

    void execute(domain::DurationMinutes minutes);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETTIMERDURATIONUSECASE_HPP
