#ifndef ICONTROLCHARTSTABVIEW_HPP
#define ICONTROLCHARTSTABVIEW_HPP

#include <string>

namespace presentation::controlChartsTab {

class IControlChartsTabView {
  public:
    virtual ~IControlChartsTabView() = default;

    virtual void setMinutes(int minutes) = 0;
    virtual void setMinutesInputEnabled(bool enabled) = 0;
    virtual void setOperatorDurationVisible(bool visible) = 0;
    virtual void setEstimatedDurationVisible(bool visible) = 0;
    virtual void setEstimatedDurationText(const std::string &text) = 0;
    virtual void setTestProtocolMode(const std::string &mode) = 0;
    virtual void setTestProtocolProgram(const std::string &program) = 0;

    virtual void setBeaufort(double value) = 0;
    virtual void setDirection(double value) = 0;
    virtual void setAngleOfAttack(double value) = 0;
    virtual void setUseAngleOfAttackModel(bool enabled) = 0;

    virtual void refreshPlot() = 0;
    virtual void showReadinessMessage(const std::string &message) = 0;
    virtual void appendLog(const std::string &text) = 0;
};

} // namespace presentation::controlChartsTab

#endif // ICONTROLCHARTSTABVIEW_HPP
