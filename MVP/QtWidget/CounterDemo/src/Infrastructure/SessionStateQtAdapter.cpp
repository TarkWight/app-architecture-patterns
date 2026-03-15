#include "SessionStateQtAdapter.hpp"

namespace infrastructure {

SessionStateQtAdapter::SessionStateQtAdapter(application::session::SessionState &state, QObject *parent)
    : QObject(parent), state(state) {
    subscription = state.subscribe([this](const application::session::SessionStateData &data) { emitState(data); });
}

QColor SessionStateQtAdapter::toQColor(domain::RgbColor color) {
    return QColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b));
}

void SessionStateQtAdapter::emitState(const application::session::SessionStateData &data) {
    emit timerChanged(data.elapsed.value, data.timerRunning);
    emit timerDurationChanged(data.timerDuration.value);

    emit functionExpressionChanged(QString::fromStdString(data.functionExpression));
    emit lineColorChanged(toQColor(data.lineColor));

    emit tab2MinutesChanged(data.tab2Minutes.value);

    emit poemTitleChanged(QString::fromStdString(data.poem.title));

    for (int i = 0; i < 8; ++i) {
        emit poemLineChanged(i, QString::fromStdString(data.poem.lines[static_cast<std::size_t>(i)]));
    }

    emit plot1Changed();
    emit plot2Changed();
}

} // namespace infrastructure
