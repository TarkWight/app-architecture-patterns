#ifndef ITAB3VIEW_HPP
#define ITAB3VIEW_HPP

#include <string>

namespace presentation::tab3 {

class ITab3View {
  public:
    virtual ~ITab3View() = default;

    virtual void setTimerDurationMinutes(int minutes) = 0;

    virtual void setPoemTitle(const std::string &title) = 0;
    virtual void setPoemLine(int index, const std::string &line) = 0;

    virtual void showExportSuccess(const std::string &filePath) = 0;
    virtual void appendLog(const std::string &text) = 0;
};

} // namespace presentation::tab3

#endif // ITAB3VIEW_HPP
