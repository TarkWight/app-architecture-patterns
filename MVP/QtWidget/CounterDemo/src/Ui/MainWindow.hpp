#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/IShellView.hpp"
#include "../Presentation/ShellPresenter.hpp"
#include "../Presentation/TelemetryChartsTab/TelemetryChartsTabPresenter.hpp"
#include "../Presentation/ControlChartsTab/ControlChartsTabPresenter.hpp"
#include "../Presentation/TestProtocolTab/TestProtocolTabPresenter.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

namespace ui {

class TelemetryChartsTabWidget;
class ControlChartsTabWidget;
class TestProtocolTabWidget;

class MainWindow final : public QMainWindow, public presentation::IShellView {
    Q_OBJECT

  public:
    struct Dependencies {
        presentation::ShellPresenter &shellPresenter;
        presentation::telemetryChartsTab::TelemetryChartsTabPresenter &telemetryChartsTabPresenter;
        presentation::controlChartsTab::ControlChartsTabPresenter &controlChartsTabPresenter;
        presentation::testProtocolTab::TestProtocolTabPresenter &testProtocolTabPresenter;
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
    presentation::telemetryChartsTab::TelemetryChartsTabPresenter &telemetryChartsTabPresenter;
    presentation::controlChartsTab::ControlChartsTabPresenter &controlChartsTabPresenter;
    presentation::testProtocolTab::TestProtocolTabPresenter &testProtocolTabPresenter;
    infrastructure::SessionStateQtAdapter &sessionAdapter;

    TelemetryChartsTabWidget *telemetryChartsTabWidget{nullptr};
    ControlChartsTabWidget *controlChartsTabWidget{nullptr};
    TestProtocolTabWidget *testProtocolTabWidget{nullptr};

    void setupTabs();
    void connectShellSignals();
    void connectSessionSignals();
};

} // namespace ui

#endif // MAINWINDOW_HPP
