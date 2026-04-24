#ifndef SETFUNCTIONEXPRESSIONUSECASE_HPP
#define SETFUNCTIONEXPRESSIONUSECASE_HPP

#include "../Session/SessionState.hpp"

#include <string>

namespace application::useCases {

class SetFunctionExpressionUseCase final {
  public:
    explicit SetFunctionExpressionUseCase(application::session::SessionState &state);

    void execute(std::string expression);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // SETFUNCTIONEXPRESSIONUSECASE_HPP
