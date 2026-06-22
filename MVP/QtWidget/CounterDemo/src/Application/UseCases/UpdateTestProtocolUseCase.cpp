#include "UpdateTestProtocolUseCase.hpp"

#include "../../Domain/TestModeStatePolicy.hpp"

namespace application::useCases {

UpdateTestProtocolUseCase::UpdateTestProtocolUseCase(application::session::SessionState &state) : state(state) {
}

void UpdateTestProtocolUseCase::updateTitle(std::string title) {
    state.setTestProtocolTitle(std::move(title));
}

void UpdateTestProtocolUseCase::updateLine(int index, std::string line) {
    state.setTestProtocolLine(index, std::move(line));
}

void UpdateTestProtocolUseCase::updateMode(std::string mode) {
    const auto testMode = domain::testModeFromKey(mode);
    const auto modeState = domain::TestModeStatePolicy::fromTestMode(testMode);

    state.setTestModeState(modeState.testMode, modeState.standControlMode, modeState.timeSource,
                           modeState.timeDirection);
}

void UpdateTestProtocolUseCase::updateProgram(std::string program) {
    state.setTestProtocolProgram(domain::testProgramFromKey(program));
}

void UpdateTestProtocolUseCase::updateDroneParameterValue(int index, std::string value) {
    state.setTestProtocolDroneParameterValue(index, std::move(value));
}

} // namespace application::useCases
