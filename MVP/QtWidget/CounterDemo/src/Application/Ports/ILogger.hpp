#ifndef ILOGGER_HPP
#define ILOGGER_HPP

#include <string>

namespace application::ports {

class ILogger {
  public:
    virtual ~ILogger() = default;
    virtual void info(const std::string &message) = 0;
};

} // namespace application::ports

#endif // ILOGGER_HPP
