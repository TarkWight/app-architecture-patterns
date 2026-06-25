#ifndef SETUSEANGLEOFATTACKMODELUSECASE_HPP
#define SETUSEANGLEOFATTACKMODELUSECASE_HPP

#include "../Session/SessionState.hpp"

namespace application::useCases {

class SetUseAngleOfAttackModelUseCase final {
  public:
    explicit SetUseAngleOfAttackModelUseCase(application::session::SessionState &state);

    void execute(bool enabled);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETUSEANGLEOFATTACKMODELUSECASE_HPP
