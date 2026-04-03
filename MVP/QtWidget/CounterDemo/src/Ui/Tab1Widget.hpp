#ifndef TAB1WIDGET_HPP
#define TAB1WIDGET_HPP

#include <QWidget>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/Tab1/ITab1View.hpp"
#include "../Presentation/Tab1/Tab1Presenter.hpp"

#include "PlotWidget.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class Tab1Widget;
}
QT_END_NAMESPACE

namespace ui {

class Tab1Widget final : public QWidget, public presentation::tab1::ITab1View {
    Q_OBJECT

  public:
    explicit Tab1Widget(presentation::tab1::Tab1Presenter &presenter,
                        infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent = nullptr);
    ~Tab1Widget() override;

    void refreshPlot() override;
    void appendLog(const std::string &text) override;

  private:
    Ui::Tab1Widget *ui;
    presentation::tab1::Tab1Presenter &presenter;
    infrastructure::SessionStateQtAdapter &sessionAdapter;
    PlotWidget *plotWidget{nullptr};

    void connectSignals();
    void connectSessionSignals();
};

} // namespace ui

#endif // TAB1WIDGET_HPP
