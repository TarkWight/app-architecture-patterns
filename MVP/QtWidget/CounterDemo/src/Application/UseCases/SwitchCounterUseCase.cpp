#include "SwitchCounterUseCase.hpp"
#include <format>
#include <stdexcept>

namespace application::useCases {

SwitchCounterUseCase::SwitchCounterUseCase(ports::ICounterRepository &repository, ports::ILogger &logger)
    : repository(repository), logger(logger) {
}

domain::CounterId SwitchCounterUseCase::tabId(int tabIndex) const {
    return domain::CounterId{static_cast<domain::CounterId::Rep>(tabIndex)};
}

domain::CounterId SwitchCounterUseCase::globalId() const {
    return domain::CounterId{static_cast<domain::CounterId::Rep>(3)}; // 0-1-2 are tabs | 3 - global
}

int SwitchCounterUseCase::loadValue(domain::CounterId id) {
    auto counterOpt = repository.getId(id);
    if (!counterOpt.has_value()) {
        throw std::runtime_error("Counter not found");
    }
    return counterOpt->getValue();
}

application::dto::CountersSnapshot SwitchCounterUseCase::snapshot() {
    return dto::CountersSnapshot{.tab0 = loadValue(tabId(0)),
                                 .tab1 = loadValue(tabId(1)),
                                 .tab2 = loadValue(tabId(2)),
                                 .global = loadValue(globalId())};
}

dto::CountersSnapshot SwitchCounterUseCase::execute(domain::CounterMode mode, int activeTabIndex,
                                                    const domain::CounterCommand &command) {
    const auto targetId = (mode == domain::CounterMode::Global) ? globalId() : tabId(activeTabIndex);

    auto counterOpt = repository.getId(targetId);
    if (!counterOpt.has_value()) {
        throw std::runtime_error("Counter not found");
    }

    auto counter = *counterOpt;
    counter.apply(command);
    repository.save(counter);

    logger.info(std::format("UseCase: execute command for counterId={}, mode={}", counter.getId().getValue(),
                            (mode == domain::CounterMode::Global ? "Global" : "PerTab")));

    return snapshot();
}

} // namespace application::useCases
