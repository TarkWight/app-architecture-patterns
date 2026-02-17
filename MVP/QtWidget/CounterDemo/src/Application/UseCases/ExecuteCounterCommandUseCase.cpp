#include "ExecuteCounterCommandUseCase.hpp"
#include <stdexcept>
#include <string>

namespace application::useCases {

ExecuteCounterCommandUseCase::ExecuteCounterCommandUseCase(ports::ICounterRepository &repository,
                                                           ports::ILogger &logger)
    : repository(repository), logger(logger) {
}

dto::CommandResult ExecuteCounterCommandUseCase::execute(domain::CounterId counterId,
                                                         const domain::CounterCommand &command) {
    auto counterOpt = repository.getId(counterId);
    if (!counterOpt.has_value()) {
        throw std::runtime_error("Counter not found");
    }

    auto counter = *counterOpt;
    counter.apply(command);
    repository.save(counter);

    logger.info("Command executed");

    return dto::CommandResult{.newValue = counter.getValue()};
}

} // namespace application::useCases
