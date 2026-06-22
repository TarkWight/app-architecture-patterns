#ifndef GENERATESTAIRPLOTUSECASE_H
#define GENERATESTAIRPLOTUSECASE_H

#include "../Session/SessionState.hpp"
#include "../../Application/Dto/PlotModel.hpp"

namespace application::useCases {

class GenerateStairPlotUseCase final {
  public:
    explicit GenerateStairPlotUseCase(application::session::SessionState &state);

    application::dto::PlotModel execute();

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // GENERATESTAIRPLOTUSECASE_H
