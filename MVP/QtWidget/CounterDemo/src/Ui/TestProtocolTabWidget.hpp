#ifndef TAB3WIDGET_HPP
#define TAB3WIDGET_HPP

#include <QWidget>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/TestProtocolTab/ITestProtocolTabView.hpp"
#include "../Presentation/TestProtocolTab/TestProtocolTabPresenter.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class TestProtocolTabWidget;
}
QT_END_NAMESPACE

namespace ui {

class TestProtocolTabWidget final : public QWidget, public presentation::testProtocolTab::ITestProtocolTabView {
    Q_OBJECT

  public:
    explicit TestProtocolTabWidget(presentation::testProtocolTab::TestProtocolTabPresenter &presenter,
                                   infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent = nullptr);
    ~TestProtocolTabWidget() override;

    void setOperatorTestDurationMinutes(int minutes) override;
    void setTestProtocolTitle(const std::string &title) override;
    void setTestProtocolLine(int index, const std::string &line) override;
    void showExportSuccess(const std::string &filePath) override;
    void appendLog(const std::string &text) override;

  private:
    Ui::TestProtocolTabWidget *ui;
    presentation::testProtocolTab::TestProtocolTabPresenter &presenter;
    infrastructure::SessionStateQtAdapter &sessionAdapter;

    void connectSignals();
    void connectSessionSignals();
};

} // namespace ui

#endif // TAB3WIDGET_HPP
