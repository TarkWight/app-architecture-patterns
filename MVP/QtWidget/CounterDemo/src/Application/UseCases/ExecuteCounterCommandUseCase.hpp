#ifndef EXECUTECOUNTERCOMMANDUSECASE_HPP
#define EXECUTECOUNTERCOMMANDUSECASE_HPP

#include "../Ports/ICounterRepository.hpp"
#include "../Ports/ILogger.hpp"
#include "../Dto/CommandResult.hpp"
#include "../../Domain/Command.hpp"
#include "../../Domain/CounterId.hpp"

namespace application::useCases {

class ExecuteCounterCommandUseCase final {
  public:
    ExecuteCounterCommandUseCase(ports::ICounterRepository &repository, ports::ILogger &logger);

    dto::CommandResult execute(domain::CounterId counterId, const domain::CounterCommand &command);

  private:
    ports::ICounterRepository &repository;
    ports::ILogger &logger;
};

} // namespace application::useCases

#endif // EXECUTECOUNTERCOMMANDUSECASE_HPP
