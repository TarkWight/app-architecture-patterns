#ifndef TAB1PRESENTER_HPP
#define TAB1PRESENTER_HPP

#include "../../Application/UseCases/GenerateStairPlotUseCase.hpp"
#include "ITab1View.hpp"

namespace presentation::tab1 {

class Tab1Presenter final {
  public:
    Tab1Presenter(application::useCases::GenerateStairPlotUseCase &generateStairPlotUseCase);

    void attachView(ITab1View &view);
    void detachView();

    void onViewReady();

    void onRebuildPlotPressed();

  private:
    application::useCases::GenerateStairPlotUseCase &generateStairPlotUseCase;

    ITab1View *view{nullptr};
};

} // namespace presentation::tab1

#endif // TAB1PRESENTER_HPP
