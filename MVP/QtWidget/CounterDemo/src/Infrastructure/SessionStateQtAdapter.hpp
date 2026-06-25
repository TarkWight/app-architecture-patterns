#ifndef SESSIONSTATEQTADAPTER_HPP
#define SESSIONSTATEQTADAPTER_HPP

#include <QObject>
#include <QString>
#include <QColor>

#include "../Application/Session/SessionState.hpp"
#include "../Presentation/ViewModels/TestTimeViewModel.hpp"

#include <vector>

namespace infrastructure {

class SessionStateQtAdapter final : public QObject {
    Q_OBJECT

  public:
    explicit SessionStateQtAdapter(application::session::SessionState &state, QObject *parent = nullptr);
    ~SessionStateQtAdapter() override = default;

    const application::session::SessionState &getState() const;

  signals:
    void testTimeModelChanged(const presentation::viewModels::TestTimeViewModel &model);

    void functionExpressionChanged(const QString &expression);
    void lineColorChanged(const QColor &color);

    void controlChartsTabMinutesChanged(int minutes);

    void beaufortChanged(double value);
    void directionChanged(double value);
    void angleOfAttackChanged(double value);
    void useAngleOfAttackModelChanged(bool enabled);

    void testProtocolTitleChanged(const QString &title);
    void testProtocolLineChanged(int index, const QString &line);
    void testProtocolModeChanged(const QString &mode);
    void testProtocolProgramChanged(const QString &program);
    void testProtocolDroneParametersChanged(const std::vector<domain::TestProtocolParameter> &parameters);

    void telemetryPlotChanged();
    void controlPlotChanged();

  private:
    application::session::SessionState &state;
    application::session::Subscription subscription;

    static QColor toQColor(application::dto::RgbColor color);
    static presentation::viewModels::TestTimeViewModel
    toTestTimeViewModel(const application::session::SessionStateData &data);
    void emitState(const application::session::SessionStateData &data);
};

} // namespace infrastructure

#endif // SESSIONSTATEQTADAPTER_HPP
