#ifndef MAINPRESENTER_HPP
#define MAINPRESENTER_HPP

#include "IMainView.hpp"
#include "../Application/UseCases/ExecuteCounterCommandUseCase.hpp"
#include "../Domain/CounterId.hpp"

using namespace application;

namespace presentation {

class MainPresenter final {
  public:
    explicit MainPresenter(useCases::ExecuteCounterCommandUseCase &executeCommandUseCase);

    void attachView(IMainView &view);
    void detachView();

    void onViewReady();
    void onTabChanged(int tabIndex);

    void onIncrementPressed();
    void onDecrementPressed();
    void onResetPressed();

  private:
    domain::CounterId currentCounterId{0};

    useCases::ExecuteCounterCommandUseCase &executor;
    IMainView *view{nullptr};

    int stepFor(domain::CounterId counterId) const;
    void executeAndRefresh(const domain::CounterCommand &command, const std::string &logText);
};

} // namespace presentation

#endif // MAINPRESENTER_HPP
