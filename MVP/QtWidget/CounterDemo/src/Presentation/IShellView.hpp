#ifndef ISHELLVIEW_HPP
#define ISHELLVIEW_HPP

#include <string>
#include "../Domain/TestTimeSource.hpp"

namespace presentation {

class IShellView {
  public:
    virtual ~IShellView() = default;

    virtual void setTimerText(const std::string &text) = 0;
    virtual void setStartEnabled(bool enabled) = 0;
    virtual void setStopEnabled(bool enabled) = 0;
    virtual void setPauseEnabled(bool enabled) = 0;
    virtual void setResumeEnabled(bool enabled) = 0;
    virtual void setStandConnectionButtonText(const std::string &text) = 0;

    virtual void setFunctionExpression(const std::string &expression) = 0;
    virtual void setTestTimeSource(domain::TestTimeSource source) = 0;

    virtual void appendLog(const std::string &text) = 0;
};

} // namespace presentation

#endif // ISHELLVIEW_HPP
