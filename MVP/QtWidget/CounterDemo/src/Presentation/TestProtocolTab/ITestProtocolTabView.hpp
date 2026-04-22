#ifndef ITAB3VIEW_HPP
#define ITAB3VIEW_HPP

#include <string>

namespace presentation::testProtocolTab {

class ITestProtocolTabView {
  public:
    virtual ~ITestProtocolTabView() = default;

    virtual void setOperatorTestDurationMinutes(int minutes) = 0;

    virtual void setTestProtocolTitle(const std::string &title) = 0;
    virtual void setTestProtocolLine(int index, const std::string &line) = 0;

    virtual void showExportSuccess(const std::string &filePath) = 0;
    virtual void appendLog(const std::string &text) = 0;
};

} // namespace presentation::testProtocolTab

#endif // ITAB3VIEW_HPP
