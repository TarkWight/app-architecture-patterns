#ifndef ICOUNTERREPOSITORY_HPP
#define ICOUNTERREPOSITORY_HPP

#include <optional>
#include "../../Domain/Counter.hpp"
#include "../../Domain/CounterId.hpp"

namespace application::ports {

class ICounterRepository {
  public:
    virtual ~ICounterRepository() = default;

    virtual std::optional<domain::Counter> getId(domain::CounterId id) = 0;
    virtual void save(const domain::Counter &counter) = 0;
};

} // namespace application::ports

#endif // ICOUNTERREPOSITORY_HPP
