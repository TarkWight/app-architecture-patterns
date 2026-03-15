#ifndef TAB2WIDGET_HPP
#define TAB2WIDGET_HPP

#include <QWidget>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/Tab2/ITab2View.hpp"
#include "../Presentation/Tab2/Tab2Presenter.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class Tab2Widget;
}
QT_END_NAMESPACE

namespace ui {

class Tab2Widget final : public QWidget, public presentation::tab2::ITab2View {
    Q_OBJECT

  public:
    explicit Tab2Widget(presentation::tab2::Tab2Presenter &presenter,
                        infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent = nullptr);
    ~Tab2Widget() override;

    void setCounterValue(int value) override;
    void setMinutes(int minutes) override;
    void refreshPlot() override;
    void appendLog(const std::string &text) override;

  private:
    Ui::Tab2Widget *ui;
    presentation::tab2::Tab2Presenter &presenter;
    infrastructure::SessionStateQtAdapter &sessionAdapter;

    void connectSignals();
    void connectSessionSignals();
};

} // namespace ui

#endif // TAB2WIDGET_HPP
