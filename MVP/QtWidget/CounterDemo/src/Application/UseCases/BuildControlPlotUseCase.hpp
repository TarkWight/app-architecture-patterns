#ifndef BUILDCONTROLPLOTUSECASE_HPP
#define BUILDCONTROLPLOTUSECASE_HPP

#include "../Ports/IFunctionEngine.hpp"
#include "../Session/SessionState.hpp"
#include "../../Domain/Plot.hpp"

namespace application::useCases {

class BuildControlPlotUseCase final {
  public:
    BuildControlPlotUseCase(application::session::SessionState &state,
                            const application::ports::IFunctionEngine &engine);

    domain::PlotModel execute();

  private:
    application::session::SessionState &state;
    const application::ports::IFunctionEngine &engine;
};

} // namespace application::useCases

#endif // BUILDCONTROLPLOTUSECASE_HPP
