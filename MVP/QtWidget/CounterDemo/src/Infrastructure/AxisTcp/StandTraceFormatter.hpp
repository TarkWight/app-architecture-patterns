#ifndef STANDTRACEFORMATTER_HPP
#define STANDTRACEFORMATTER_HPP

#include "../../Domain/AxisControlCommand.hpp"
#include "../../Domain/AxisId.hpp"
#include "../../Domain/AxisTelemetrySample.hpp"

#include <string>

namespace infrastructure::axisTcp {

[[nodiscard]] std::string standTraceAxisName(domain::AxisId axisId);
[[nodiscard]] std::string formatStandLifecycleTrace(const std::string &event, domain::AxisId axisId,
                                                    const std::string &details = {});
[[nodiscard]] std::string formatStandTxTrace(domain::AxisId axisId, const domain::AxisControlCommand &command);
[[nodiscard]] std::string formatStandRxTrace(const domain::AxisTelemetrySample &sample);

} // namespace infrastructure::axisTcp

#endif // STANDTRACEFORMATTER_HPP
