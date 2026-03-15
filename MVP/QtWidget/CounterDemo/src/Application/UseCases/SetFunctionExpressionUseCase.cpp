#include "SetFunctionExpressionUseCase.hpp"

namespace application::useCases {

SetFunctionExpressionUseCase::SetFunctionExpressionUseCase(application::session::SessionState &state) : state(state) {
}

void SetFunctionExpressionUseCase::execute(std::string expression) {
    state.setFunctionExpression(std::move(expression));
}

} // namespace application::useCases
