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
    void setBeaufort(double value) override;
    void setDirection(double value) override;
    void setAngleOfAttack(double value) override;
    void refreshPlot() override;
    void appendLog(const std::string &text) override;

    void insertTopPanel(QWidget &panel);

  private:
    Ui::ControlChartsTabWidget *ui;
    presentation::controlChartsTab::ControlChartsTabPresenter &presenter;
    infrastructure::SessionStateQtAdapter &sessionAdapter;
    PlotWidget *plotWidget{nullptr};

    void connectSignals();
    void connectSessionSignals();
};

} // namespace ui

#endif // TAB2WIDGET_HPP
