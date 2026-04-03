#ifndef SESSIONSTATEQTADAPTER_HPP
#define SESSIONSTATEQTADAPTER_HPP

#include <QObject>
#include <QString>
#include <QColor>

#include "../Application/Session/SessionState.hpp"

namespace infrastructure {

class SessionStateQtAdapter final : public QObject {
    Q_OBJECT

  public:
    explicit SessionStateQtAdapter(application::session::SessionState &state, QObject *parent = nullptr);
    ~SessionStateQtAdapter() override = default;

    const application::session::SessionState &getState() const;

  signals:
    void timerChanged(int elapsedSeconds, bool running);
    void timerDurationChanged(int minutes);

    void functionExpressionChanged(const QString &expression);
    void lineColorChanged(const QColor &color);

    void controlChartsTabMinutesChanged(int minutes);

    void poemTitleChanged(const QString &title);
    void poemLineChanged(int index, const QString &line);

    void telemetryPlotChanged();
    void controlPlotChanged();

  private:
    application::session::SessionState &state;
    application::session::Subscription subscription;

    static QColor toQColor(domain::RgbColor color);
    void emitState(const application::session::SessionStateData &data);
};

} // namespace infrastructure

#endif // SESSIONSTATEQTADAPTER_HPP
