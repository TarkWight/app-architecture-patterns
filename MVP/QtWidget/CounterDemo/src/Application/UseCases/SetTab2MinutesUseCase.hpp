#ifndef SETTAB2MINUTESUSECASE_HPP
#define SETTAB2MINUTESUSECASE_HPP

#include "../Session/SessionState.hpp"

namespace application::useCases {

class SetTab2MinutesUseCase final {
  public:
    explicit SetTab2MinutesUseCase(application::session::SessionState &state);

    void execute(int minutes);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETTAB2MINUTESUSECASE_HPP
