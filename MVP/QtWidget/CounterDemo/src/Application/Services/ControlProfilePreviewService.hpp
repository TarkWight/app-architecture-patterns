#ifndef CONTROLPROFILEPREVIEWSERVICE_HPP
#define CONTROLPROFILEPREVIEWSERVICE_HPP

#include "../Ports/IFunctionEngine.hpp"
#include "../Session/SessionStateData.hpp"

#include "../../Domain/WindControlProfile.hpp"

namespace application::services {

class ControlProfilePreviewService final {
  public:
    domain::WindControlProfile build(const application::session::ProtocolStateData &protocol,
                                     const application::session::ControlStateData &control,
                                     const application::ports::IFunctionEngine &engine) const;
};

} // namespace application::services

#endif // CONTROLPROFILEPREVIEWSERVICE_HPP
