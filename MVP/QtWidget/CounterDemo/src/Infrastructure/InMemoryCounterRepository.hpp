#ifndef INMEMORYCOUNTERREPOSITORY_HPP
#define INMEMORYCOUNTERREPOSITORY_HPP

#include <unordered_map>
#include <optional>

#include "../Application/Ports/ICounterRepository.hpp"
#include "../Domain/Counter.hpp"
#include "../Domain/CounterId.hpp"

namespace infrastructure {

class InMemoryCounterRepository final : public application::ports::ICounterRepository {
  public:
    InMemoryCounterRepository();

    std::optional<domain::Counter> getId(domain::CounterId counterId) override;
    void save(const domain::Counter &counter) override;

  private:
    struct KeyHash {
        std::size_t operator()(domain::CounterId id) const noexcept {
            return static_cast<std::size_t>(id.getValue());
        }
    };

    std::unordered_map<domain::CounterId, domain::Counter, KeyHash> counters;
};

} // namespace infrastructure

#endif // INMEMORYCOUNTERREPOSITORY_HPP
