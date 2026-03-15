#include "UpdatePoemUseCase.hpp"

namespace application::useCases {

UpdatePoemUseCase::UpdatePoemUseCase(application::session::SessionState &state) : state(state) {
}

void UpdatePoemUseCase::updateTitle(std::string title) {
    state.setPoemTitle(std::move(title));
}

void UpdatePoemUseCase::updateLine(int index, std::string line) {
    state.setPoemLine(index, std::move(line));
}

} // namespace application::useCases
