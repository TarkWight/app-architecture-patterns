#include "InMemoryCounterRepository.hpp"

namespace infrastructure {

InMemoryCounterRepository::InMemoryCounterRepository() {
    counters.try_emplace(domain::CounterId{0}, domain::Counter(domain::CounterId{0}));
    counters.try_emplace(domain::CounterId{1}, domain::Counter(domain::CounterId{1}));
    counters.try_emplace(domain::CounterId{2}, domain::Counter(domain::CounterId{2}));
    counters.try_emplace(domain::CounterId{3}, domain::Counter{domain::CounterId{3}});
}

std::optional<domain::Counter> InMemoryCounterRepository::getId(domain::CounterId id) {
    const auto counter = counters.find(id);
    if (counter == counters.end()) {
        return std::nullopt;
    }
    return counter->second;
}

void InMemoryCounterRepository::save(const domain::Counter &counter) {
    counters.insert_or_assign(counter.getId(), counter);
}

} // namespace infrastructure
