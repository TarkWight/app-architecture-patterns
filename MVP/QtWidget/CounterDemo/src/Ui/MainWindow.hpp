#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/IShellView.hpp"
#include "../Presentation/ShellPresenter.hpp"
#include "../Presentation/Tab1/Tab1Presenter.hpp"
#include "../Presentation/Tab2/Tab2Presenter.hpp"
#include "../Presentation/Tab3/Tab3Presenter.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

namespace ui {

class Tab1Widget;
class Tab2Widget;
class Tab3Widget;

class MainWindow final : public QMainWindow, public presentation::IShellView {
    Q_OBJECT

  public:
    struct Dependencies {
        presentation::ShellPresenter &shellPresenter;
        presentation::tab1::Tab1Presenter &tab1Presenter;
        presentation::tab2::Tab2Presenter &tab2Presenter;
        presentation::tab3::Tab3Presenter &tab3Presenter;
        infrastructure::SessionStateQtAdapter &sessionAdapter;
    };

    explicit MainWindow(Dependencies deps, QWidget *parent = nullptr);
    ~MainWindow() override;

    void setTimerText(const std::string &text) override;
    void setStartEnabled(bool enabled) override;
    void setStopEnabled(bool enabled) override;
    void setFunctionExpression(const std::string &expression) override;
    void appendLog(const std::string &text) override;

  private:
    std::unique_ptr<Ui::MainWindow> ui;

    presentation::ShellPresenter &shellPresenter;
    presentation::tab1::Tab1Presenter &tab1Presenter;
    presentation::tab2::Tab2Presenter &tab2Presenter;
    presentation::tab3::Tab3Presenter &tab3Presenter;
    infrastructure::SessionStateQtAdapter &sessionAdapter;

    Tab1Widget *tab1Widget{nullptr};
    Tab2Widget *tab2Widget{nullptr};
    Tab3Widget *tab3Widget{nullptr};

    void setupTabs();
    void connectShellSignals();
    void connectSessionSignals();
};

} // namespace ui

#endif // MAINWINDOW_HPP
