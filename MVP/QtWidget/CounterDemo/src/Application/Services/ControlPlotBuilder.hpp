#ifndef CONTROLPLOTBUILDER_HPP
#define CONTROLPLOTBUILDER_HPP

#include "../Dto/PlotModel.hpp"
#include "../Session/SessionStateData.hpp"

#include "../../Domain/WindControlProfile.hpp"

namespace application::services {

class ControlPlotBuilder final {
  public:
    application::dto::PlotModel build(const application::session::SessionStateData &stateData,
                                      const domain::WindControlProfile &profile) const;
};

} // namespace application::services

#endif // CONTROLPLOTBUILDER_HPP
