#ifndef ITAB2VIEW_HPP
#define ITAB2VIEW_HPP

#include <string>

namespace presentation::controlChartsTab {

class IControlChartsTabView {
  public:
    virtual ~IControlChartsTabView() = default;

    virtual void setMinutes(int minutes) = 0;
    virtual void refreshPlot() = 0;
    virtual void appendLog(const std::string &text) = 0;
};

} // namespace presentation::controlChartsTab

#endif // ITAB2VIEW_HPP
