#include "LoadPdfReportDefaultsUseCase.hpp"

#include <array>
#include <utility>

namespace application::useCases {

LoadPdfReportDefaultsUseCase::LoadPdfReportDefaultsUseCase(application::session::SessionState &state,
                                                           application::ports::IConfigRepository &configRepository)
    : state(state), configRepository(configRepository) {
}

void LoadPdfReportDefaultsUseCase::execute(const std::string &configPath) {
    const auto config = configRepository.loadPdfReportConfig(configPath);

    state.setTestProtocolTitle(config.title);

    const std::array<std::string, 8> lines{
        config.organization, config.licenseNumber, config.address, config.operatorName,
        config.comment,      config.conclusion,    config.result,  {}};

    for (std::size_t i = 0; i < lines.size(); ++i) {
        state.setTestProtocolLine(static_cast<int>(i), lines[i]);
    }

    state.setTestProtocolMode(config.testMode);
    state.setTestProtocolProgram(config.testProgram);
    state.setTestProtocolDroneParameters(config.droneParameters);
}

} // namespace application::useCases
