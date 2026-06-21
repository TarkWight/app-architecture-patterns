#ifndef ESTIMATETESTDURATIONUSECASE_HPP
#define ESTIMATETESTDURATIONUSECASE_HPP

#include "../Session/SessionState.hpp"

#include "../Ports/IFunctionEngine.hpp"

#include "../../Domain/TestDurationEstimator.hpp"

namespace application::useCases {

class EstimateTestDurationUseCase final {
  public:
    explicit EstimateTestDurationUseCase(application::session::SessionState &state);
    EstimateTestDurationUseCase(application::session::SessionState &state,
                                const application::ports::IFunctionEngine &engine);

    domain::EstimatedTestDurationResult executeForAutoCalculated();
    domain::EstimatedTestDurationResult executeForImpact(domain::WindImpact impact,
                                                         bool calculatedForWorstCaseScenario = false);

  private:
    application::session::SessionState &state;
    const application::ports::IFunctionEngine *engine{nullptr};
};

} // namespace application::useCases

#endif // ESTIMATETESTDURATIONUSECASE_HPP
