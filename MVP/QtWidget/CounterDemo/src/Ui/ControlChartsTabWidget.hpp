#ifndef TAB2WIDGET_HPP
#define TAB2WIDGET_HPP

#include <QWidget>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/ControlChartsTab/IControlChartsTabView.hpp"
#include "../Presentation/ControlChartsTab/ControlChartsTabPresenter.hpp"
#include "PlotWidget.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class ControlChartsTabWidget;
}
QT_END_NAMESPACE

namespace ui {

class ControlChartsTabWidget final : public QWidget, public presentation::controlChartsTab::IControlChartsTabView {
    Q_OBJECT

  public:
    explicit ControlChartsTabWidget(presentation::controlChartsTab::ControlChartsTabPresenter &presenter,
                                    infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent = nullptr);
    ~ControlChartsTabWidget() override;

    void setMinutes(int minutes) override;
    void setMinutesInputEnabled(bool enabled) override;
    void setOperatorDurationVisible(bool visible) override;
    void setEstimatedDurationVisible(bool visible) override;
    void setEstimatedDurationText(const std::string &text) override;
    void setTestProtocolMode(const std::string &mode) override;
    void setTestProtocolProgram(const std::string &program) override;
    void setBeaufort(double value) override;
    void setDirection(double value) override;
    void setAngleOfAttack(double value) override;
    void refreshPlot() override;
    void showReadinessMessage(const std::string &message) override;
    void appendLog(const std::string &text) override;
    void setFunctionExpression(const std::string &expression);

  signals:
    void functionEdited(const QString &expression);
    void formulaTemplateSelected(const QString &key);
    void calculateRequested();
    void lineColorRequested();
    void logMessage(const QString &text);

  private:
    Ui::ControlChartsTabWidget *ui;
    presentation::controlChartsTab::ControlChartsTabPresenter &presenter;
    infrastructure::SessionStateQtAdapter &sessionAdapter;
    PlotWidget *plotWidget{nullptr};

    void connectSignals();
    void connectSessionSignals();
    void populateFormulaTemplates();
    void populateTestSelectionControls();
    void updateFormulaTemplateSelection(const std::string &expression);
};

} // namespace ui

#endif // TAB2WIDGET_HPP
