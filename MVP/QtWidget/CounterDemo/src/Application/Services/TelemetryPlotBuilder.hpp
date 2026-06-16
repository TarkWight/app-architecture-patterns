#ifndef TELEMETRYPLOTBUILDER_HPP
#define TELEMETRYPLOTBUILDER_HPP

#include "../Dto/PlotModel.hpp"
#include "../Session/SessionStateData.hpp"

namespace application::services {

class TelemetryPlotBuilder final {
  public:
    application::dto::PlotModel build(const application::session::SessionStateData &stateData) const;
};

} // namespace application::services

#endif // TELEMETRYPLOTBUILDER_HPP
