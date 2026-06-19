#ifndef CONFIGURETELEMETRYUSECASE_HPP
#define CONFIGURETELEMETRYUSECASE_HPP

#include "../Ports/IConfigRepository.hpp"
#include "../Ports/ITelemetryClient.hpp"
#include "../Services/TelemetrySessionClock.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class ConfigureTelemetryUseCase {
  public:
    ConfigureTelemetryUseCase(application::session::SessionState &state,
                              application::ports::IConfigRepository &configRepository,
                              application::ports::ITelemetryClient &telemetryClient);
    ConfigureTelemetryUseCase(application::session::SessionState &state,
                              application::ports::IConfigRepository &configRepository,
                              application::ports::ITelemetryClient &telemetryClient,
                              application::services::TelemetrySessionClock &telemetrySessionClock);

    void execute(const std::string &configPath);

  private:
    application::session::SessionState &state;
    application::ports::IConfigRepository &configRepository;
    application::ports::ITelemetryClient &telemetryClient;
    application::services::TelemetrySessionClock *telemetrySessionClock{nullptr};
};

} // namespace application::useCases

#endif // CONFIGURETELEMETRYUSECASE_HPP
