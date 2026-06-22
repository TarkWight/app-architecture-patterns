#include "SessionStateQtAdapter.hpp"

namespace infrastructure {

SessionStateQtAdapter::SessionStateQtAdapter(application::session::SessionState &state, QObject *parent)
    : QObject(parent), state(state) {
    subscription = state.subscribe([this](const application::session::SessionStateData &data) { emitState(data); });
}

QColor SessionStateQtAdapter::toQColor(application::dto::RgbColor color) {
    return QColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b));
}

presentation::viewModels::TestTimeViewModel
SessionStateQtAdapter::toTestTimeViewModel(const application::session::SessionStateData &data) {

    presentation::viewModels::TestTimeViewModel model{};
    model.executionStatus = data.execution.testExecutionStatus;
    model.timeSource = data.protocol.testTimeSource;
    model.timeDirection = data.execution.testTimeDirection;

    model.estimatedDurationMinutes = data.protocol.estimatedTestDuration.value();
    model.operatorDurationMinutes = data.protocol.operatorTestDuration.value();
    model.activeDurationMinutes = data.execution.activeTestDuration.value();

    model.elapsedSeconds = data.execution.elapsed.value();
    model.remainingSeconds = data.execution.remaining.value();

    model.displayedSeconds = (data.execution.testTimeDirection == domain::TestTimeDirection::CountDown)
                                 ? data.execution.remaining.value()
                                 : data.execution.elapsed.value();

    return model;
}

void SessionStateQtAdapter::emitState(const application::session::SessionStateData &data) {
    emit testTimeModelChanged(toTestTimeViewModel(data));

    emit functionExpressionChanged(QString::fromStdString(data.control.functionExpression.value));
    emit lineColorChanged(toQColor(data.control.lineColor));

    emit controlChartsTabMinutesChanged(data.control.controlChartsTabMinutes.value());

    emit beaufortChanged(data.control.windImpact.beaufort.value());
    emit directionChanged(data.control.windImpact.direction.degrees());
    emit angleOfAttackChanged(data.control.windImpact.angleOfAttack.degrees());

    emit testProtocolTitleChanged(QString::fromStdString(data.protocol.testProtocol.title));
    emit testProtocolModeChanged(QString::fromUtf8(domain::testModeKey(data.protocol.testProtocol.testMode).data()));
    emit testProtocolProgramChanged(
        QString::fromUtf8(domain::testProgramKey(data.protocol.testProtocol.testProgram).data()));
    emit testProtocolDroneParametersChanged(data.protocol.testProtocol.droneParameters);

    for (int i = 0; i < 8; ++i) {
        emit testProtocolLineChanged(
            i, QString::fromStdString(data.protocol.testProtocol.lines[static_cast<std::size_t>(i)]));
    }

    emit telemetryPlotChanged();
    emit controlPlotChanged();
}

const application::session::SessionState &SessionStateQtAdapter::getState() const {
    return state;
}

} // namespace infrastructure
