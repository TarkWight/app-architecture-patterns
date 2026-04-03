#include "UpdateTestProtocolUseCase.hpp"

namespace application::useCases {

UpdateTestProtocolUseCase::UpdateTestProtocolUseCase(application::session::SessionState &state) : state(state) {
}

void UpdateTestProtocolUseCase::updateTitle(std::string title) {
    state.setTestProtocolTitle(std::move(title));
}

void UpdateTestProtocolUseCase::updateLine(int index, std::string line) {
    state.setTestProtocolLine(index, std::move(line));
}

} // namespace application::useCases
