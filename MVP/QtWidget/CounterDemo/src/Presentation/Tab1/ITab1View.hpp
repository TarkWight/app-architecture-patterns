#ifndef ITAB1VIEW_HPP
#define ITAB1VIEW_HPP

#include <string>

namespace presentation::tab1 {

class ITab1View {
  public:
    virtual ~ITab1View() = default;

    virtual void refreshPlot() = 0;
    virtual void appendLog(const std::string &text) = 0;
};

} // namespace presentation::tab1

#endif // ITAB1VIEW_HPP
