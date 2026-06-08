#include "UpdateTestProtocolUseCase.hpp"

#include "../../Domain/StandControlMode.hpp"
#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"

namespace application::useCases {

namespace {

void applyTimeRulesForTestMode(application::session::SessionState &state, domain::TestMode mode) {
    switch (mode) {
    case domain::TestMode::Manual:
        state.setTestTimeSource(domain::TestTimeSource::FreeRun);
        state.setTestTimeDirection(domain::TestTimeDirection::CountUp);
        break;
    case domain::TestMode::Hybrid:
    case domain::TestMode::Automatic:
        state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
        state.setTestTimeDirection(domain::TestTimeDirection::CountDown);
        break;
    }
}

} // namespace

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

    state.setTestProtocolMode(testMode);
    state.setStandControlMode(domain::standControlModeForTestMode(testMode));
    applyTimeRulesForTestMode(state, testMode);
}

void UpdateTestProtocolUseCase::updateProgram(std::string program) {
    state.setTestProtocolProgram(domain::testProgramFromKey(program));
}

void UpdateTestProtocolUseCase::updateDroneParameterValue(int index, std::string value) {
    state.setTestProtocolDroneParameterValue(index, std::move(value));
}

} // namespace application::useCases
