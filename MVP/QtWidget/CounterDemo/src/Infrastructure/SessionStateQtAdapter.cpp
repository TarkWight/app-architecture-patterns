#include "SessionStateQtAdapter.hpp"

namespace infrastructure {

SessionStateQtAdapter::SessionStateQtAdapter(application::session::SessionState &state, QObject *parent)
    : QObject(parent), state(state) {
    subscription = state.subscribe([this](const application::session::SessionStateData &data) { emitState(data); });
}

QColor SessionStateQtAdapter::toQColor(domain::RgbColor color) {
    return QColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b));
}

presentation::viewModels::TestTimeViewModel
SessionStateQtAdapter::toTestTimeViewModel(const application::session::SessionStateData &data) {

    presentation::viewModels::TestTimeViewModel model{};
    model.executionStatus = data.testExecutionStatus;
    model.timeSource = data.testTimeSource;
    model.timeDirection = data.testTimeDirection;

    model.estimatedDurationMinutes = data.estimatedTestDuration.value;
    model.operatorDurationMinutes = data.operatorTestDuration.value;
    model.activeDurationMinutes = data.activeTestDuration.value;

    model.elapsedSeconds = data.elapsed.value;
    model.remainingSeconds = data.remaining.value;

    model.displayedSeconds =
        (data.testTimeDirection == domain::TestTimeDirection::CountDown) ? data.remaining.value : data.elapsed.value;

    return model;
}

void SessionStateQtAdapter::emitState(const application::session::SessionStateData &data) {
    emit testTimeModelChanged(toTestTimeViewModel(data));

    emit functionExpressionChanged(QString::fromStdString(data.functionExpression.value));
    emit lineColorChanged(toQColor(data.lineColor));

    emit controlChartsTabMinutesChanged(data.controlChartsTabMinutes.value);

    emit beaufortChanged(data.windProfile.beaufort.value());
    emit directionChanged(data.windProfile.direction.degrees());
    emit angleOfAttackChanged(data.windProfile.angleOfAttack.degrees());

    emit testProtocolTitleChanged(QString::fromStdString(data.testProtocol.title));
    emit testProtocolModeChanged(QString::fromUtf8(domain::testModeKey(data.testProtocol.testMode).data()));
    emit testProtocolProgramChanged(QString::fromUtf8(domain::testProgramKey(data.testProtocol.testProgram).data()));
    emit testProtocolDroneParametersChanged(data.testProtocol.droneParameters);

    for (int i = 0; i < 8; ++i) {
        emit testProtocolLineChanged(i, QString::fromStdString(data.testProtocol.lines[static_cast<std::size_t>(i)]));
    }

    emit telemetryPlotChanged();
    emit controlPlotChanged();
}

const application::session::SessionState &SessionStateQtAdapter::getState() const {
    return state;
}

} // namespace infrastructure
