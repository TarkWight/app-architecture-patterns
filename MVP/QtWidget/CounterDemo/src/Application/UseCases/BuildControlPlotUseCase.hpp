#ifndef BUILDCONTROLPLOTUSECASE_HPP
#define BUILDCONTROLPLOTUSECASE_HPP

#include "EstimateTestDurationUseCase.hpp"

#include "../Services/ControlProfilePreviewService.hpp"
#include "../Services/ControlPlotBuilder.hpp"
#include "../Ports/IFunctionEngine.hpp"
#include "../Session/SessionState.hpp"
#include "../../Application/Dto/PlotModel.hpp"

namespace application::useCases {

class BuildControlPlotUseCase final {
  public:
    BuildControlPlotUseCase(application::session::SessionState &state,
                            const application::ports::IFunctionEngine &engine);

    application::dto::PlotModel execute();
    application::dto::PlotModel refreshFromState();

  private:
    application::session::SessionState &state;
    const application::ports::IFunctionEngine &engine;
    EstimateTestDurationUseCase estimateTestDurationUseCase;
    application::services::ControlProfilePreviewService profilePreviewService{};
    application::services::ControlPlotBuilder plotBuilder{};
};

} // namespace application::useCases

#endif // BUILDCONTROLPLOTUSECASE_HPP
