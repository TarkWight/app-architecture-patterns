#ifndef CONFIGURETELEMETRYUSECASE_HPP
#define CONFIGURETELEMETRYUSECASE_HPP

#include "../Ports/IConfigRepository.hpp"
#include "../Ports/ITelemetryClient.hpp"

namespace application::useCases {

class ConfigureTelemetryUseCase {
public:
    ConfigureTelemetryUseCase(
        application::ports::IConfigRepository &configRepository,
        application::ports::ITelemetryClient &telemetryClient
        );

    void execute(const std::string &configPath);
private:
    application::ports::IConfigRepository &configRepository;
    application::ports::ITelemetryClient &telemetryClient;
};

} // namespace application::useCases

#endif // CONFIGURETELEMETRYUSECASE_HPP