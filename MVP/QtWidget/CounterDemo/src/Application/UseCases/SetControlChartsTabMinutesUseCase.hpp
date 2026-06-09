#ifndef SETTAB2MINUTESUSECASE_HPP
#define SETTAB2MINUTESUSECASE_HPP

#include "../Session/SessionState.hpp"

#include "../../Domain/TestTimeSource.hpp"

namespace application::useCases {

class SetControlChartsTabMinutesUseCase final {
  public:
    explicit SetControlChartsTabMinutesUseCase(application::session::SessionState &state);

    void execute(int minutes);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETTAB2MINUTESUSECASE_HPP
