#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>
#include <string>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/IShellView.hpp"
#include "../Presentation/ShellPresenter.hpp"
#include "../Presentation/TelemetryChartsTab/TelemetryChartsTabPresenter.hpp"
#include "../Presentation/ControlChartsTab/ControlChartsTabPresenter.hpp"
#include "../Presentation/TestProtocolTab/TestProtocolTabPresenter.hpp"
#include "../Application/UseCases/SetStandControlModeUseCase.hpp"
#include "../Application/UseCases/SetStandImpactUseCase.hpp"
#include "../Domain/AxisId.hpp"
#include "../Domain/TestTimeSource.hpp"

class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QString;
class QTimer;
class QVBoxLayout;
class QWidget;

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
        application::useCases::SetStandControlModeUseCase &setStandControlModeUseCase;
        application::useCases::SetStandImpactUseCase &setStandImpactUseCase;
        infrastructure::SessionStateQtAdapter &sessionAdapter;
    };

    explicit MainWindow(Dependencies deps, QWidget *parent = nullptr);
    ~MainWindow() override;

    void setTimerText(const std::string &text) override;
    void setStartEnabled(bool enabled) override;
    void setStopEnabled(bool enabled) override;
    void setPauseResumeEnabled(bool enabled) override;
    void setPauseResumeText(const std::string &text) override;
    void setStandConnectionButtonText(const std::string &text) override;
    void setStandConnectionStatusText(const std::string &text) override;
    void setTestTimeSource(domain::TestTimeSource source) override;
    void setTestTimeSourceEnabled(bool enabled) override;

    void setFunctionExpression(const std::string &expression) override;

    void appendLog(const std::string &text) override;
    void showOperatorWarning(const std::string &title, const std::string &message) override;

  private:
    std::unique_ptr<Ui::MainWindow> ui;

    presentation::ShellPresenter &shellPresenter;
    presentation::telemetryChartsTab::TelemetryChartsTabPresenter &telemetryChartsTabPresenter;
    presentation::controlChartsTab::ControlChartsTabPresenter &controlChartsTabPresenter;
    presentation::testProtocolTab::TestProtocolTabPresenter &testProtocolTabPresenter;
    application::useCases::SetStandControlModeUseCase &setStandControlModeUseCase;
    application::useCases::SetStandImpactUseCase &setStandImpactUseCase;
    infrastructure::SessionStateQtAdapter &sessionAdapter;

    TelemetryChartsTabWidget *telemetryChartsTabWidget{nullptr};
    ControlChartsTabWidget *controlChartsTabWidget{nullptr};
    TestProtocolTabWidget *testProtocolTabWidget{nullptr};

    QDoubleSpinBox *standBeaufortSpinBox{nullptr};
    QDoubleSpinBox *standAngleOfAttackSpinBox{nullptr};
    QComboBox *standDirectionComboBox{nullptr};
    QComboBox *standControlModeComboBox{nullptr};
    QComboBox *telemetrySourceComboBox{nullptr};
    QCheckBox *telemetryCurveVisibleCheckBox{nullptr};
    QComboBox *controlFormulaTemplateComboBox{nullptr};
    QLineEdit *controlFormulaLineEdit{nullptr};
    QTimer *standImpactTransitionTimer{nullptr};
    QPushButton *standApplyButton{nullptr};
    QLabel *standConnectionStatusLabel{nullptr};
    bool controlPlotRebuildScheduled{false};
    std::string observedTestProtocolModeKey{};

    void setupTabs();
    void setupStandConnectionStatusIndicator();
    void setupMainContentLayout();
    QWidget *createStandControlPanel();
    QWidget *createControlFormulaPanel();
    QLabel *createPanelTitle(QWidget &parent, const QString &text) const;
    void addStandImpactRows(QFormLayout &form, QWidget &parent);
    void addTelemetryBindingRows(QFormLayout &form, QWidget &parent);
    void addStandControlButtons(QVBoxLayout &layout, QWidget &parent);
    void connectShellSignals();
    void connectSessionSignals();
    void applyStandInputs();
    void advanceStandImpactTransition();
    void selectTelemetryAxisColor();
    void scheduleControlPlotRebuild();
    void updateStandControlModeSelection();
    void updateManualStandControlsEnabled();
    void updateControlFormulaTemplateSelection(const std::string &expression);
    double selectedStandDirectionDegrees() const;
    domain::AxisId selectedTelemetryAxisId() const;
};

} // namespace ui

#endif // MAINWINDOW_HPP
