#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>

#include "../Presentation/IMainView.hpp"
#include "../Presentation/MainPresenter.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
} // namespace Ui
QT_END_NAMESPACE

namespace ui {

class MainWindow final : public QMainWindow, public presentation::IMainView {
    Q_OBJECT

  public:
    explicit MainWindow(presentation::MainPresenter &presenter, QWidget *parent = nullptr);
    ~MainWindow();

    void setCounterValue(domain::CounterId counterId, int value) override;
    void appendCommandLog(const std::string &text) override;

  private:
    std::unique_ptr<Ui::MainWindow> ui;
    presentation::MainPresenter &presenter;

    void connectSignals();
};

} // namespace ui

#endif // MAINWINDOW_HPP
