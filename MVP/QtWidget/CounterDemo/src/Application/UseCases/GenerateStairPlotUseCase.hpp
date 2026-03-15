#ifndef GENERATESTAIRPLOTUSECASE_H
#define GENERATESTAIRPLOTUSECASE_H

#include "../Session/SessionState.hpp"
#include "../../Domain/Plot.hpp"

namespace application::useCases {

class GenerateStairPlotUseCase final {
  public:
    explicit GenerateStairPlotUseCase(application::session::SessionState &state);

    domain::PlotModel execute();

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // GENERATESTAIRPLOTUSECASE_H
