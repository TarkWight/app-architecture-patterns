#ifndef CONTROLFUNCTIONWORSTCASEANALYZER_HPP
#define CONTROLFUNCTIONWORSTCASEANALYZER_HPP

#include "../Ports/IFunctionEngine.hpp"
#include "../Session/SessionStateData.hpp"

#include "../../Domain/Time.hpp"
#include "../../Domain/WindImpact.hpp"

#include <string>
#include <vector>

namespace application::services {

constexpr int defaultControlFunctionAnalysisWindowMinutes = 48;

struct WorstCaseImpactCandidate {
    domain::WindImpact impact{};
    bool applicable{false};
    bool usedFunction{false};
    std::vector<std::string> diagnostics{};
};

class ControlFunctionWorstCaseAnalyzer final {
  public:
    [[nodiscard]] WorstCaseImpactCandidate analyze(const session::ProtocolStateData &protocol,
                                                   const session::ControlStateData &control,
                                                   const ports::IFunctionEngine &engine,
                                                   domain::DurationMinutes analysisDuration) const;
};

} // namespace application::services

#endif // CONTROLFUNCTIONWORSTCASEANALYZER_HPP
