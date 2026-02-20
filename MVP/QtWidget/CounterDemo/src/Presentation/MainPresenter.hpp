#ifndef MAINPRESENTER_HPP
#define MAINPRESENTER_HPP

#include "IMainView.hpp"
#include "../Application/UseCases/SwitchCounterUseCase.hpp"
#include "../Domain/CounterMode.hpp"

namespace presentation {

class MainPresenter final {
  public:
    explicit MainPresenter(application::useCases::SwitchCounterUseCase &switchCounterUseCase);

    void attachView(IMainView &view);
    void detachView();

    void onViewReady();
    void onTabChanged(int tabIndex);

    void onModeToggled(bool isGlobal);

    void onIncrementPressed();
    void onDecrementPressed();
    void onResetPressed();

  private:
    int activeTabIndex{0};
    domain::CounterMode mode{domain::CounterMode::PerTab};

    application::useCases::SwitchCounterUseCase &useCase;
    IMainView *view{nullptr};

    int stepForTab(int tabIndex) const;

    void refreshUi(const application::dto::CountersSnapshot &snapshot);
    void executeAndRefresh(const domain::CounterCommand &command, const std::string &logText);
};

} // namespace presentation

#endif // MAINPRESENTER_HPP
