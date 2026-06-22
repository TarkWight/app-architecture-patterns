#include "StandTraceFormatter.hpp"

#include <iomanip>
#include <sstream>

namespace infrastructure::axisTcp {

namespace {

std::string number(double value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(3) << value;
    return out.str();
}

std::string flags(const domain::AxisControlCommand &command) {
    std::ostringstream out;
    out << "(" << (command.cmd1 ? 1 : 0) << "," << (command.cmd2 ? 1 : 0) << "," << (command.cmd3 ? 1 : 0) << ","
        << (command.cmd4 ? 1 : 0) << ")";
    return out.str();
}

} // namespace

std::string standTraceAxisName(domain::AxisId axisId) {
    return axisId == domain::axis0 ? "axis0" : "axis1";
}

std::string formatStandLifecycleTrace(const std::string &event, domain::AxisId axisId, const std::string &details) {
    std::string message = "[STAND][" + event + "][" + standTraceAxisName(axisId) + "]";
    if (!details.empty()) {
        message += " " + details;
    }
    return message;
}

std::string formatStandTxTrace(domain::AxisId axisId, const domain::AxisControlCommand &command) {
    return "[STAND][TX][" + standTraceAxisName(axisId) + "] pos=" + number(command.position) +
           ", vel=" + number(command.velocity) + ", torque=" + number(command.torque) + ", flags=" + flags(command);
}

std::string formatStandRxTrace(const domain::AxisTelemetrySample &sample) {
    return "[STAND][RX][" + standTraceAxisName(sample.axisId) + "] pos=" + number(sample.position) +
           ", setPos=" + number(sample.setPosition) + ", torque=" + number(sample.torque) +
           ", voltage=" + number(sample.voltage) + ", ibus=" + number(sample.current);
}

} // namespace infrastructure::axisTcp
