#ifndef TAB3WIDGET_HPP
#define TAB3WIDGET_HPP

#include <QWidget>

#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Presentation/TestProtocolTab/ITestProtocolTabView.hpp"
#include "../Presentation/TestProtocolTab/TestProtocolTabPresenter.hpp"

#include <vector>

class QComboBox;
class QGridLayout;
class QLineEdit;

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
    void setTestProtocolMode(const std::string &mode) override;
    void setTestProtocolProgram(const std::string &program) override;
    void setTestProtocolDroneParameters(const std::vector<domain::TestProtocolParameter> &parameters) override;
    void showExportSuccess(const std::string &filePath) override;
    void appendLog(const std::string &text) override;

  private:
    Ui::TestProtocolTabWidget *ui;
    presentation::testProtocolTab::TestProtocolTabPresenter &presenter;
    infrastructure::SessionStateQtAdapter &sessionAdapter;
    QComboBox *testModeComboBox{nullptr};
    QComboBox *testProgramComboBox{nullptr};
    QGridLayout *droneParametersLayout{nullptr};
    std::vector<QLineEdit *> droneParameterEdits{};

    void connectSignals();
    void connectSessionSignals();
    void setupReportFormLabels();
    void setupTestSelectionControls();
    void setupDroneParametersEditor();
};

} // namespace ui

#endif // TAB3WIDGET_HPP
