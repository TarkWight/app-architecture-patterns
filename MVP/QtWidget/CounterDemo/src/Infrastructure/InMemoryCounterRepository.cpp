#include "InMemoryCounterRepository.hpp"

namespace infrastructure {

InMemoryCounterRepository::InMemoryCounterRepository() {
    counters.emplace(domain::CounterId{0}, domain::Counter(domain::CounterId{0}));
    counters.emplace(domain::CounterId{1}, domain::Counter(domain::CounterId{1}));
    counters.emplace(domain::CounterId{2}, domain::Counter(domain::CounterId{2}));
}

std::optional<domain::Counter> InMemoryCounterRepository::getId(domain::CounterId id) {
    auto counter = counters.find(id);
    if (counter == counters.end()) {
        return std::nullopt;
    }
    return counter->second;
}

void InMemoryCounterRepository::save(const domain::Counter &counter) {
    counters.insert_or_assign(counter.getId(), counter);
}

} // namespace infrastructure
