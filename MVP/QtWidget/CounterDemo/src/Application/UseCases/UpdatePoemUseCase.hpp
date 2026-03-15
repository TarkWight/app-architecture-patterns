#ifndef UPDATEPOEMUSECASE_HPP
#define UPDATEPOEMUSECASE_HPP

#include "../Session/SessionState.hpp"

#include <string>

namespace application::useCases {

class UpdatePoemUseCase final {
  public:
    explicit UpdatePoemUseCase(application::session::SessionState &state);

    void updateTitle(std::string title);
    void updateLine(int index, std::string line);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // UPDATEPOEMUSECASE_HPP
