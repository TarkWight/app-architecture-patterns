#ifndef CALCULATEANDBUILDCONTROLPLOTUSECASE_HPP
#define CALCULATEANDBUILDCONTROLPLOTUSECASE_HPP

#include "BuildControlPlotUseCase.hpp"
#include "EstimateTestDurationUseCase.hpp"

#include "../Ports/IFunctionEngine.hpp"
#include "../Services/ControlFunctionWorstCaseAnalyzer.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class CalculateAndBuildControlPlotUseCase final {
  public:
    CalculateAndBuildControlPlotUseCase(application::session::SessionState &state,
                                        const application::ports::IFunctionEngine &engine,
                                        BuildControlPlotUseCase &buildControlPlotUseCase);

    void execute();

  private:
    application::session::SessionState &state;
    const application::ports::IFunctionEngine &engine;
    BuildControlPlotUseCase &buildControlPlotUseCase;
    EstimateTestDurationUseCase estimateTestDurationUseCase;
    application::services::ControlFunctionWorstCaseAnalyzer analyzer{};

    [[nodiscard]] domain::DurationMinutes analysisDuration() const;
};

} // namespace application::useCases

#endif // CALCULATEANDBUILDCONTROLPLOTUSECASE_HPP
