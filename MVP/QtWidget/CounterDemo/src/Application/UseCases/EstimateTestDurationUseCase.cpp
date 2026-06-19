#include "EstimateTestDurationUseCase.hpp"

#include "../Services/UavSpecificationMapper.hpp"

#include "../../Domain/TestTimeSource.hpp"

namespace application::useCases {

namespace {

bool hasExplicitImpact(const domain::WindImpact &impact) {
    return impact.beaufort.value() > 0.0 || impact.direction.degrees() > 0.0 || impact.angleOfAttack.degrees() != 0.0;
}

domain::WindImpact impactForEstimation(const application::session::ControlStateData &control) {
    if (hasExplicitImpact(control.targetStandImpact)) {
        return control.targetStandImpact;
    }

    return control.windImpact;
}

} // namespace

EstimateTestDurationUseCase::EstimateTestDurationUseCase(application::session::SessionState &state) : state(state) {
}

domain::EstimatedTestDurationResult EstimateTestDurationUseCase::executeForAutoCalculated() {
    const auto &protocol = state.protocol();
    if (protocol.testTimeSource != domain::TestTimeSource::AutoCalculated) {
        state.resetReadiness();
        return {};
    }

    const auto impact = impactForEstimation(state.control());
    const auto uavSpecification = application::services::UavSpecificationMapper{}.map(protocol.testProtocol);
    if (!uavSpecification.has_value()) {
        domain::EstimatedTestDurationResult result{};
        state.setReadinessFromEstimationResult(result, impact);
        return result;
    }

    const auto result = domain::TestDurationEstimator::estimate(domain::TestDurationEstimationContext{
        .uav = *uavSpecification,
        .impact = impact,
    });

    if (result.duration.has_value()) {
        state.setEstimatedTestDurationMinutes(*result.duration);
    }

    state.setReadinessFromEstimationResult(result, impact);
    return result;
}

} // namespace application::useCases
