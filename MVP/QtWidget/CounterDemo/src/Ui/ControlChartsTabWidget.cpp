#include "ControlChartsTabWidget.hpp"
#include "ui_ControlChartsTabWidget.h"

#include "../Domain/FormulaTemplate.hpp"
#include "../Domain/TestProtocol.hpp"
#include "../Domain/TestTimeSource.hpp"
#include "../Domain/WindImpact.hpp"

#include <QSignalBlocker>
#include <QString>

namespace ui {

ControlChartsTabWidget::ControlChartsTabWidget(presentation::controlChartsTab::ControlChartsTabPresenter &presenter,
                                               infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent)
    : QWidget(parent), ui(new Ui::ControlChartsTabWidget), presenter(presenter), sessionAdapter(sessionAdapter) {
    ui->setupUi(this);
    ui->doubleSpinBoxBeaufort->setRange(domain::minOperationalBeaufort, domain::maxOperationalBeaufort);
    populateFormulaTemplates();

    ui->labelBeaufortCaption->hide();
    ui->doubleSpinBoxBeaufort->hide();
    ui->labelDirectionCaption->hide();
    ui->doubleSpinBoxDirection->hide();
    ui->labelAngleOfAttackCaption->hide();
    ui->doubleSpinBoxAngleOfAttack->hide();

    plotWidget = new PlotWidget(this);
    ui->verticalLayoutPlot->replaceWidget(ui->labelPlotState, plotWidget);
    ui->labelPlotState->hide();

    presenter.attachView(*this);

    connectSignals();
    connectSessionSignals();
    updateMinutesInputEnabled();
    setFunctionExpression(sessionAdapter.getState().get().functionExpression.value);
}

ControlChartsTabWidget::~ControlChartsTabWidget() {
    presenter.detachView();
    delete ui;
}

void ControlChartsTabWidget::setMinutes(int minutes) {
    const QSignalBlocker blocker{ui->spinBoxMinutes};
    ui->spinBoxMinutes->setValue(minutes);
}

void ControlChartsTabWidget::setBeaufort(double value) {
    const QSignalBlocker blocker{ui->doubleSpinBoxBeaufort};
    ui->doubleSpinBoxBeaufort->setValue(value);
}

void ControlChartsTabWidget::setDirection(double value) {
    const QSignalBlocker blocker{ui->doubleSpinBoxDirection};
    ui->doubleSpinBoxDirection->setValue(value);
}

void ControlChartsTabWidget::setAngleOfAttack(double value) {
    const QSignalBlocker blocker{ui->doubleSpinBoxAngleOfAttack};
    ui->doubleSpinBoxAngleOfAttack->setValue(value);
}

void ControlChartsTabWidget::refreshPlot() {
    plotWidget->setPlot(sessionAdapter.getState().get().controlPlot);
}

void ControlChartsTabWidget::appendLog(const std::string &text) {
    emit logMessage(QString::fromStdString(text));
}

void ControlChartsTabWidget::setFunctionExpression(const std::string &expression) {
    updateFormulaTemplateSelection(expression);

    if (ui->lineEditFormula->hasFocus() || ui->lineEditFormula->text().toStdString() == expression) {
        return;
    }

    const QSignalBlocker blocker{ui->lineEditFormula};
    ui->lineEditFormula->setText(QString::fromStdString(expression));
}

void ControlChartsTabWidget::connectSignals() {
    QObject::connect(ui->lineEditFormula, &QLineEdit::editingFinished, this,
                     [this]() { emit functionEdited(ui->lineEditFormula->text()); });

    QObject::connect(ui->comboBoxFormulaTemplate, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index <= 0) {
            return;
        }

        emit formulaTemplateSelected(ui->comboBoxFormulaTemplate->currentData().toString());
    });

    QObject::connect(ui->buttonCalculatePlot, &QPushButton::clicked, this, [this]() { emit calculateRequested(); });
    QObject::connect(ui->buttonPickLineColor, &QPushButton::clicked, this, [this]() { emit lineColorRequested(); });

    QObject::connect(ui->spinBoxMinutes, qOverload<int>(&QSpinBox::valueChanged), this,
                     [this](int value) { presenter.onMinutesChanged(value); });

    QObject::connect(ui->doubleSpinBoxBeaufort, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                     [this](double value) { presenter.onBeaufortChanged(value); });

    QObject::connect(ui->doubleSpinBoxDirection, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                     [this](double value) { presenter.onDirectionChanged(value); });

    QObject::connect(ui->doubleSpinBoxAngleOfAttack, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                     [this](double value) { presenter.onAngleOfAttackChanged(value); });
}

void ControlChartsTabWidget::connectSessionSignals() {
    QObject::connect(
        &sessionAdapter, &infrastructure::SessionStateQtAdapter::testTimeModelChanged, this,
        [this](const presentation::viewModels::TestTimeViewModel & /*model*/) { updateMinutesInputEnabled(); });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::testProtocolModeChanged, this,
                     [this](const QString & /*mode*/) { updateMinutesInputEnabled(); });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::controlChartsTabMinutesChanged, this,
                     [this](int minutes) {
                         if (ui->spinBoxMinutes->value() == minutes) {
                             return;
                         }

                         setMinutes(minutes);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::beaufortChanged, this,
                     [this](double value) {
                         if (ui->doubleSpinBoxBeaufort->value() == value) {
                             return;
                         }

                         setBeaufort(value);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::directionChanged, this,
                     [this](double value) {
                         if (ui->doubleSpinBoxDirection->value() == value) {
                             return;
                         }

                         setDirection(value);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::angleOfAttackChanged, this,
                     [this](double value) {
                         if (ui->doubleSpinBoxAngleOfAttack->value() == value) {
                             return;
                         }

                         setAngleOfAttack(value);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::controlPlotChanged, this,
                     [this]() { refreshPlot(); });
}

void ControlChartsTabWidget::updateMinutesInputEnabled() {
    const auto &state = sessionAdapter.getState().get();
    const bool enabled = state.testProtocol.testMode == domain::TestMode::Hybrid &&
                         state.testTimeSource == domain::TestTimeSource::OperatorDefined;
    ui->spinBoxMinutes->setEnabled(enabled);
}

void ControlChartsTabWidget::populateFormulaTemplates() {
    ui->comboBoxFormulaTemplate->addItem(QStringLiteral("Своя формула"), QString{});

    for (const auto &formulaTemplate : domain::formulaTemplates) {
        ui->comboBoxFormulaTemplate->addItem(
            QString::fromUtf8(formulaTemplate.title.data(), static_cast<qsizetype>(formulaTemplate.title.size())),
            QString::fromUtf8(formulaTemplate.key.data(), static_cast<qsizetype>(formulaTemplate.key.size())));
    }
}

void ControlChartsTabWidget::updateFormulaTemplateSelection(const std::string &expression) {
    const QSignalBlocker blocker{ui->comboBoxFormulaTemplate};
    const auto key = domain::formulaTemplateKeyByExpression(expression);
    const int index =
        key.empty()
            ? 0
            : ui->comboBoxFormulaTemplate->findData(QString::fromUtf8(key.data(), static_cast<qsizetype>(key.size())));
    ui->comboBoxFormulaTemplate->setCurrentIndex(index >= 0 ? index : 0);
}

} // namespace ui
