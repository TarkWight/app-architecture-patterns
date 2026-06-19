#ifndef READINESSDIAGNOSTICMESSAGEBUILDER_HPP
#define READINESSDIAGNOSTICMESSAGEBUILDER_HPP

#include "../Session/SessionStateData.hpp"

#include <string>
#include <vector>

namespace application::services {

struct ReadinessDiagnosticMessage {
    std::string summary{};
    std::vector<std::string> details{};

    [[nodiscard]] std::string toDisplayText() const;
};

class ReadinessDiagnosticMessageBuilder final {
  public:
    [[nodiscard]] static ReadinessDiagnosticMessage build(const session::ReadinessStateData &readiness);
    [[nodiscard]] static std::string messageForDiagnostic(domain::TestDurationDiagnosticCode code,
                                                          const domain::TestDurationDiagnosticValues &values);
};

} // namespace application::services

#endif // READINESSDIAGNOSTICMESSAGEBUILDER_HPP
