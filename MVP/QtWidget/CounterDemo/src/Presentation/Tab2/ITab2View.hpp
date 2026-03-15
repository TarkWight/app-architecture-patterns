#ifndef ITAB2VIEW_HPP
#define ITAB2VIEW_HPP

#include <string>

namespace presentation::tab2 {

class ITab2View {
  public:
    virtual ~ITab2View() = default;

    virtual void setCounterValue(int value) = 0;
    virtual void setMinutes(int minutes) = 0;
    virtual void refreshPlot() = 0;
    virtual void appendLog(const std::string &text) = 0;
};

} // namespace presentation::tab2

#endif // ITAB2VIEW_HPP
