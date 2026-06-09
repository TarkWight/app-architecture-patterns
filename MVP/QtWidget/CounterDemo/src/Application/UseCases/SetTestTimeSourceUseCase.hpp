#ifndef SETTESTTIMESOURCEUSECASE_HPP
#define SETTESTTIMESOURCEUSECASE_HPP

#include "../Session/SessionState.hpp"
#include "../../Domain/TestTimeSource.hpp"

namespace application::useCases {

class SetTestTimeSourceUseCase final {
  public:
    explicit SetTestTimeSourceUseCase(application::session::SessionState &state);

    void execute(domain::TestTimeSource source);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETTESTTIMESOURCEUSECASE_HPP