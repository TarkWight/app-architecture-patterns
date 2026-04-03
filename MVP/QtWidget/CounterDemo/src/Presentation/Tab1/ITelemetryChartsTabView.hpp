#ifndef ITELEMETRYCHARTSTABVIEW_H
#define ITELEMETRYCHARTSTABVIEW_H

#include <string>

namespace presentation::tab1 {

class ITelemetryChartsTabView {
  public:
    virtual ~ITelemetryChartsTabView() = default;

    virtual void refreshPlot() = 0;
    virtual void appendLog(const std::string &text) = 0;
};

} // namespace presentation::tab1

#endif // ITELEMETRYCHARTSTABVIEW_H
