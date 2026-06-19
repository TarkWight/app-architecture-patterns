#ifndef ESTIMATETESTDURATIONUSECASE_HPP
#define ESTIMATETESTDURATIONUSECASE_HPP

#include "../Session/SessionState.hpp"

#include "../../Domain/TestDurationEstimator.hpp"

namespace application::useCases {

class EstimateTestDurationUseCase final {
  public:
    explicit EstimateTestDurationUseCase(application::session::SessionState &state);

    domain::EstimatedTestDurationResult executeForAutoCalculated();

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // ESTIMATETESTDURATIONUSECASE_HPP
