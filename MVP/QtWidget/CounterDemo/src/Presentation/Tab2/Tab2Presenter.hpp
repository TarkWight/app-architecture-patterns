#ifndef TAB2PRESENTER_HPP
#define TAB2PRESENTER_HPP

#include "../../Application/UseCases/BuildFormulaPlotUseCase.hpp"
#include "../../Application/UseCases/SetTab2MinutesUseCase.hpp"

#include "ITab2View.hpp"

namespace presentation::tab2 {

class Tab2Presenter final {
  public:
    struct Dependencies {
        application::session::SessionState &state;
        application::useCases::SetTab2MinutesUseCase &setTab2MinutesUseCase;
        application::useCases::BuildFormulaPlotUseCase &buildFormulaPlotUseCase;
    };

    explicit Tab2Presenter(Dependencies deps);

    void attachView(ITab2View &view);
    void detachView();

    void onViewReady();

    void onMinutesChanged(int minutes);
    void onRebuildPlotPressed();

  private:
    application::session::SessionState &state;
    application::useCases::SetTab2MinutesUseCase &setTab2MinutesUseCase;
    application::useCases::BuildFormulaPlotUseCase &buildFormulaPlotUseCase;

    ITab2View *view{nullptr};
};

} // namespace presentation::tab2

#endif // TAB2PRESENTER_HPP
