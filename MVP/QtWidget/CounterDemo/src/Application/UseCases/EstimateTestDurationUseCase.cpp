#include "EstimateTestDurationUseCase.hpp"

#include "../Services/ControlFunctionWorstCaseAnalyzer.hpp"
#include "../Services/ControlProfilePreviewService.hpp"
#include "../Services/UavSpecificationMapper.hpp"

#include "../../Domain/TestTimeSource.hpp"
#include "../../Domain/HybridBeaufortOverride.hpp"
#include "../../Domain/SafeWindImpactLimitEstimator.hpp"
#include "../../Domain/TestModeStatePolicy.hpp"
#include "../../Domain/WindControlProfileWorstCase.hpp"

namespace application::useCases {

namespace {

domain::DurationMinutes analysisDurationFor(const application::session::ProtocolStateData &protocol) {
    if (protocol.testTimeSource == domain::TestTimeSource::OperatorDefined) {
        return protocol.operatorTestDuration;
    }

    return domain::DurationMinutes::required(application::services::defaultControlFunctionAnalysisWindowMinutes);
}

bool hasExplicitImpact(const domain::WindImpact &impact) {
    return impact.beaufort.value() > 0.0 || impact.direction.degrees() > 0.0 || impact.angleOfAttack.degrees() != 0.0;
}

domain::WindImpact impactForEstimation(const application::session::ControlStateData &control) {
    if (hasExplicitImpact(control.targetStandImpact)) {
        return control.targetStandImpact;
    }

    return control.windImpact;
}

domain::WindImpact hybridImpactForEstimation(const domain::WindControlProfile &profile,
                                             const application::session::ControlStateData &control,
                                             bool &calculatedForWorstCaseScenario) {
    const auto baseImpact = impactForEstimation(control);
    auto worstCase = domain::WindControlProfileWorstCase::from(profile, baseImpact);
    auto beaufort = worstCase.impact.beaufort;
    calculatedForWorstCaseScenario = worstCase.usedProfile;

    if (control.hybridBeaufortOverride.has_value() &&
        control.hybridBeaufortOverride->operatorBeaufort.value() > beaufort.value()) {
        beaufort = control.hybridBeaufortOverride->operatorBeaufort;
        calculatedForWorstCaseScenario = true;
    }

    return domain::WindImpact{.beaufort = beaufort,
                              .direction = control.hybridOperatorDirection,
                              .angleOfAttack = control.hybridOperatorAngleOfAttack};
}

domain::WindControlProfile profileForEstimation(const application::session::ProtocolStateData &protocol,
                                                const application::session::ControlStateData &control,
                                                const application::ports::IFunctionEngine *engine) {
    if (engine == nullptr) {
        return control.controlProfile;
    }

    return application::services::ControlProfilePreviewService{}.build(protocol, control, *engine);
}

} // namespace

EstimateTestDurationUseCase::EstimateTestDurationUseCase(application::session::SessionState &state) : state(state) {
}

EstimateTestDurationUseCase::EstimateTestDurationUseCase(application::session::SessionState &state,
                                                         const application::ports::IFunctionEngine &engine)
    : state(state), engine(&engine) {
}

domain::EstimatedTestDurationResult EstimateTestDurationUseCase::executeForAutoCalculated() {
    const auto &protocol = state.protocol();
    if (protocol.testTimeSource != domain::TestTimeSource::AutoCalculated) {
        state.resetReadiness();
        return {};
    }

    const auto &control = state.control();
    const auto baseImpact = impactForEstimation(control);
    auto impact = baseImpact;
    bool calculatedForWorstCaseScenario = false;

    if (engine != nullptr && domain::TestModeStatePolicy::usesControlProfile(protocol.testProtocol.testMode)) {
        const auto candidate = application::services::ControlFunctionWorstCaseAnalyzer{}.analyze(
            protocol, control, *engine, analysisDurationFor(protocol));
        impact = candidate.impact;
        calculatedForWorstCaseScenario = candidate.usedFunction;
    } else if (protocol.testProtocol.testMode == domain::TestMode::Hybrid) {
        const auto profile = profileForEstimation(protocol, control, engine);
        impact = hybridImpactForEstimation(profile, control, calculatedForWorstCaseScenario);
    } else if (domain::TestModeStatePolicy::usesControlProfile(protocol.testProtocol.testMode)) {
        const auto profile = profileForEstimation(protocol, control, engine);
        const auto worstCase = domain::WindControlProfileWorstCase::from(profile, baseImpact);
        impact = worstCase.impact;
        calculatedForWorstCaseScenario = worstCase.usedProfile;
    }

    return executeForImpact(impact, calculatedForWorstCaseScenario);
}

domain::EstimatedTestDurationResult EstimateTestDurationUseCase::executeForImpact(domain::WindImpact impact,
                                                                                  bool calculatedForWorstCaseScenario) {
    const auto &protocol = state.protocol();
    const auto uavSpecification = application::services::UavSpecificationMapper{}.map(protocol.testProtocol);
    if (!uavSpecification.has_value()) {
        domain::EstimatedTestDurationResult result{};
        state.setReadinessFromEstimationResult(result, impact, calculatedForWorstCaseScenario);
        return result;
    }

    const auto result = domain::TestDurationEstimator::estimate(domain::TestDurationEstimationContext{
        .uav = *uavSpecification,
        .impact = impact,
    });
    const auto safeLimits = domain::SafeWindImpactLimitEstimator::estimate(*uavSpecification, impact);

    if (result.duration.has_value()) {
        state.setEstimatedTestDurationMinutes(*result.duration);
    }

    state.setReadinessFromEstimationResult(result, impact, calculatedForWorstCaseScenario, safeLimits);
    return result;
}

} // namespace application::useCases
