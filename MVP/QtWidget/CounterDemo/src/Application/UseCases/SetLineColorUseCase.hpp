#ifndef SETLINECOLORUSECASE_HPP
#define SETLINECOLORUSECASE_HPP

#include "../Session/SessionState.hpp"
#include "../../Domain/Plot.hpp"

namespace application::useCases {

class SetLineColorUseCase final {
  public:
    explicit SetLineColorUseCase(application::session::SessionState &state);

    void execute(domain::RgbColor color);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETLINECOLORUSECASE_HPP
