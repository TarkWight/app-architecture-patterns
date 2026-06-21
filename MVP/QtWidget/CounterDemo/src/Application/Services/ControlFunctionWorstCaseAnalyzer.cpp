#include "ControlFunctionWorstCaseAnalyzer.hpp"

#include "../../Domain/TestModeStatePolicy.hpp"
#include "../../Domain/WindControlProfile.hpp"

#include <algorithm>
#include <cmath>

namespace application::services {

namespace {

bool hasExplicitImpact(const domain::WindImpact &impact) {
    return impact.beaufort.value() > 0.0 || impact.direction.degrees() > 0.0 || impact.angleOfAttack.degrees() != 0.0;
}

domain::WindImpact baseImpactFor(const session::ControlStateData &control) {
    if (hasExplicitImpact(control.targetStandImpact)) {
        return control.targetStandImpact;
    }

    return control.windImpact;
}

domain::WindImpact impactBaseForMode(const session::ProtocolStateData &protocol,
                                     const session::ControlStateData &control) {
    const auto baseImpact = baseImpactFor(control);
    if (protocol.testProtocol.testMode == domain::TestMode::Hybrid) {
        return domain::WindImpact{.beaufort = baseImpact.beaufort,
                                  .direction = control.hybridOperatorDirection,
                                  .angleOfAttack = control.hybridOperatorAngleOfAttack};
    }

    return baseImpact;
}

} // namespace

WorstCaseImpactCandidate ControlFunctionWorstCaseAnalyzer::analyze(const session::ProtocolStateData &protocol,
                                                                   const session::ControlStateData &control,
                                                                   const ports::IFunctionEngine &engine,
                                                                   domain::DurationMinutes analysisDuration) const {
    WorstCaseImpactCandidate result{};
    result.impact = impactBaseForMode(protocol, control);

    if (!domain::TestModeStatePolicy::usesControlProfile(protocol.testProtocol.testMode)) {
        return result;
    }

    result.applicable = true;

    double maxBeaufort = result.impact.beaufort.value();
    const int sampleCount = static_cast<int>(static_cast<double>(std::max(1, analysisDuration.value()) * 60) /
                                             domain::windControlProfileSampleIntervalSeconds);

    for (int index = 0; index < sampleCount; ++index) {
        const double timeSeconds = static_cast<double>(index) * domain::windControlProfileSampleIntervalSeconds;
        const double timeMinutes = timeSeconds / 60.0;
        const double rawBeaufort = engine.eval(control.functionExpression.value, timeMinutes);

        if (!std::isfinite(rawBeaufort)) {
            result.diagnostics.push_back("Функция управляющего воздействия вернула нечисловое значение.");
            continue;
        }

        const auto beaufort = domain::Beaufort::from(rawBeaufort);
        if (beaufort.value() > maxBeaufort) {
            maxBeaufort = beaufort.value();
            result.impact = result.impact.withBeaufort(beaufort);
            result.usedFunction = true;
        }
    }

    if (protocol.testProtocol.testMode == domain::TestMode::Hybrid && control.hybridBeaufortOverride.has_value() &&
        control.hybridBeaufortOverride->operatorBeaufort.value() > maxBeaufort) {
        result.impact = result.impact.withBeaufort(control.hybridBeaufortOverride->operatorBeaufort);
        result.usedFunction = true;
    }

    return result;
}

} // namespace application::services
