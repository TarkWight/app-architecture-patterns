#ifndef SWITCHCOUNTERUSECASE_HPP
#define SWITCHCOUNTERUSECASE_HPP

#include "../Ports/ICounterRepository.hpp"
#include "../Ports/ILogger.hpp"
#include "../Dto/CounterSnapshot.hpp"
#include "../../Domain/Command.hpp"
#include "../../Domain/CounterId.hpp"
#include "../../Domain/CounterMode.hpp"

namespace application::useCases {

class SwitchCounterUseCase final {
  public:
    SwitchCounterUseCase(ports::ICounterRepository &repository, ports::ILogger &logger);

    dto::CountersSnapshot execute(domain::CounterMode mode, int activeTabIndex, const domain::CounterCommand &command);

    dto::CountersSnapshot snapshot();

  private:
    ports::ICounterRepository &repository;
    ports::ILogger &logger;

    domain::CounterId tabId(int tabIndex) const;
    domain::CounterId globalId() const;
    int loadValue(domain::CounterId id);
};

} // namespace application::useCases

#endif // SWITCHCOUNTERUSECASE_HPP
