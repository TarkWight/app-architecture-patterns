#ifndef TAB3WIDGET_HPP
#define TAB3WIDGET_HPP

#include <QWidget>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/Tab3/ITab3View.hpp"
#include "../Presentation/Tab3/Tab3Presenter.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class Tab3Widget;
}
QT_END_NAMESPACE

namespace ui {

class Tab3Widget final : public QWidget, public presentation::tab3::ITab3View {
    Q_OBJECT

  public:
    explicit Tab3Widget(presentation::tab3::Tab3Presenter &presenter,
                        infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent = nullptr);
    ~Tab3Widget() override;

    void setCounterValue(int value) override;
    void setTimerDurationMinutes(int minutes) override;
    void setPoemTitle(const std::string &title) override;
    void setPoemLine(int index, const std::string &line) override;
    void showExportSuccess(const std::string &filePath) override;
    void appendLog(const std::string &text) override;

  private:
    Ui::Tab3Widget *ui;
    presentation::tab3::Tab3Presenter &presenter;
    infrastructure::SessionStateQtAdapter &sessionAdapter;

    void connectSignals();
    void connectSessionSignals();
};

} // namespace ui

#endif // TAB3WIDGET_HPP
