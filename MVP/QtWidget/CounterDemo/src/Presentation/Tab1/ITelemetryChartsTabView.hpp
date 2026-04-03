#ifndef ITELEMETRYCHARTSTABVIEW_H
#define ITELEMETRYCHARTSTABVIEW_H

#include <string>

namespace presentation::telemetryChartsTab {

class ITelemetryChartsTabView {
  public:
    virtual ~ITelemetryChartsTabView() = default;

    virtual void refreshPlot() = 0;
    virtual void appendLog(const std::string &text) = 0;
};

} // namespace presentation::telemetryChartsTab

#endif // ITELEMETRYCHARTSTABVIEW_H
