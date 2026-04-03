#ifndef TELEMETRYCHARTSTABWIDGET_H
#define TELEMETRYCHARTSTABWIDGET_H

#include <QWidget>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/TelemetryChartsTab/ITelemetryChartsTabView.hpp"
#include "../Presentation/TelemetryChartsTab/TelemetryChartsTabPresenter.hpp"

#include "PlotWidget.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class TelemetryChartsTabWidget;
}
QT_END_NAMESPACE

namespace ui {

class TelemetryChartsTabWidget final : public QWidget,
                                       public presentation::telemetryChartsTab::ITelemetryChartsTabView {
    Q_OBJECT

  public:
    explicit TelemetryChartsTabWidget(presentation::telemetryChartsTab::TelemetryChartsTabPresenter &presenter,
                                      infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent = nullptr);
    ~TelemetryChartsTabWidget() override;

    void refreshPlot() override;
    void appendLog(const std::string &text) override;

  private:
    Ui::TelemetryChartsTabWidget *ui;
    presentation::telemetryChartsTab::TelemetryChartsTabPresenter &presenter;
    infrastructure::SessionStateQtAdapter &sessionAdapter;
    PlotWidget *plotWidget{nullptr};

    void connectSignals();
    void connectSessionSignals();
};

} // namespace ui

#endif // TELEMETRYCHARTSTABWIDGET_H
