#ifndef QTTEXTLOGGER_HPP
#define QTTEXTLOGGER_HPP

#include <functional>
#include <string>

#include "../Application/Ports/ILogger.hpp"

namespace infrastructure {

class QtTextLogger final : public application::ports::ILogger {
  public:
    using Sink = std::function<void(const std::string &)>;

    explicit QtTextLogger(Sink sink);

    void info(const std::string &message) override;

  private:
    Sink sink;
};

} // namespace infrastructure

#endif // QTTEXTLOGGER_HPP
