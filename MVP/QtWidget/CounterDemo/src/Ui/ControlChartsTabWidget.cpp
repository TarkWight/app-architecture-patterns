#include "ControlChartsTabWidget.hpp"
#include "ui_ControlChartsTabWidget.h"

#include "../Domain/TestProtocol.hpp"
#include "../Domain/WindImpact.hpp"
#include "../Localization/UiStrings.hpp"

#include <QSignalBlocker>
#include <QString>

namespace ui {

namespace {

QString uiText(const char *text) {
    return QString::fromUtf8(text);
}

} // namespace

ControlChartsTabWidget::ControlChartsTabWidget(presentation::controlChartsTab::ControlChartsTabPresenter &presenter,
                                               infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent)
    : QWidget(parent), ui(new Ui::ControlChartsTabWidget), presenter(presenter), sessionAdapter(sessionAdapter) {
    ui->setupUi(this);
    ui->doubleSpinBoxBeaufort->setRange(domain::minOperationalBeaufort, domain::maxOperationalBeaufort);
    populateTestSelectionControls();
    populateTestProgramSelection();

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
    presenter.onTimeSettingsChanged();
    setFunctionExpression(sessionAdapter.getState().get().control.functionExpression.value);
}

ControlChartsTabWidget::~ControlChartsTabWidget() {
    presenter.detachView();
    delete ui;
}

void ControlChartsTabWidget::setMinutes(int minutes) {
    const QSignalBlocker blocker{ui->spinBoxMinutes};
    ui->spinBoxMinutes->setValue(minutes);
}

void ControlChartsTabWidget::setMinutesInputEnabled(bool enabled) {
    ui->spinBoxMinutes->setEnabled(enabled);
}

void ControlChartsTabWidget::setOperatorDurationVisible(bool visible) {
    ui->labelMinutesCaption->setVisible(visible);
    ui->spinBoxMinutes->setVisible(visible);
}

void ControlChartsTabWidget::setEstimatedDurationVisible(bool visible) {
    ui->labelEstimatedDurationCaption->setVisible(visible);
    ui->labelEstimatedDurationValue->setVisible(visible);
}

void ControlChartsTabWidget::setEstimatedDurationText(const std::string &text) {
    ui->labelEstimatedDurationValue->setText(QString::fromStdString(text));
}

void ControlChartsTabWidget::setTestProtocolMode(const std::string &mode) {
    const QSignalBlocker blocker{ui->comboBoxTestMode};
    const int index = ui->comboBoxTestMode->findData(QString::fromStdString(mode));
    ui->comboBoxTestMode->setCurrentIndex(index >= 0 ? index : 0);
}

void ControlChartsTabWidget::setTestProtocolProgram(const std::string &program) {
    const QSignalBlocker blocker{ui->comboBoxFormulaTemplate};
    const int index = ui->comboBoxFormulaTemplate->findData(QString::fromStdString(program));
    ui->comboBoxFormulaTemplate->setCurrentIndex(index >= 0 ? index : 0);
    updateFormulaEditability(program);
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

void ControlChartsTabWidget::setUseAngleOfAttackModel(bool enabled) {
    const QSignalBlocker blocker{ui->checkBoxUseAngleOfAttackModel};
    ui->checkBoxUseAngleOfAttackModel->setChecked(enabled);
}

void ControlChartsTabWidget::refreshPlot() {
    plotWidget->setPlot(sessionAdapter.getState().get().control.controlPlot);
}

void ControlChartsTabWidget::showReadinessMessage(const std::string &message) {
    ui->plainTextEditReadinessStatus->setPlainText(QString::fromStdString(message));
}

void ControlChartsTabWidget::appendLog(const std::string &text) {
    emit logMessage(QString::fromStdString(text));
}

void ControlChartsTabWidget::setFunctionExpression(const std::string &expression) {
    if ((!ui->lineEditFormula->isReadOnly() && ui->lineEditFormula->hasFocus()) ||
        ui->lineEditFormula->text().toStdString() == expression) {
        return;
    }

    const QSignalBlocker blocker{ui->lineEditFormula};
    ui->lineEditFormula->setText(QString::fromStdString(expression));
}

void ControlChartsTabWidget::connectSignals() {
    QObject::connect(ui->lineEditFormula, &QLineEdit::editingFinished, this,
                     [this]() { emit functionEdited(ui->lineEditFormula->text()); });

    QObject::connect(ui->comboBoxFormulaTemplate, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index < 0) {
            return;
        }

        const auto program = ui->comboBoxFormulaTemplate->currentData().toString();
        updateFormulaEditability(program.toStdString());
        presenter.onTestProtocolProgramChanged(program.toStdString());
    });

    QObject::connect(ui->buttonCalculatePlot, &QPushButton::clicked, this, [this]() { emit calculateRequested(); });
    QObject::connect(ui->buttonPickLineColor, &QPushButton::clicked, this, [this]() { emit lineColorRequested(); });
    QObject::connect(ui->comboBoxTestMode, &QComboBox::currentIndexChanged, this, [this]() {
        presenter.onTestProtocolModeChanged(ui->comboBoxTestMode->currentData().toString().toStdString());
    });
    QObject::connect(ui->spinBoxMinutes, qOverload<int>(&QSpinBox::valueChanged), this,
                     [this](int value) { presenter.onMinutesChanged(value); });

    QObject::connect(ui->doubleSpinBoxBeaufort, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                     [this](double value) { presenter.onBeaufortChanged(value); });

    QObject::connect(ui->doubleSpinBoxDirection, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                     [this](double value) { presenter.onDirectionChanged(value); });

    QObject::connect(ui->doubleSpinBoxAngleOfAttack, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                     [this](double value) { presenter.onAngleOfAttackChanged(value); });

    QObject::connect(ui->checkBoxUseAngleOfAttackModel, &QCheckBox::toggled, this,
                     [this](bool enabled) { presenter.onUseAngleOfAttackModelChanged(enabled); });
}

void ControlChartsTabWidget::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::testTimeModelChanged, this,
                     [this](const presentation::viewModels::TestTimeViewModel & /*model*/) {
                         presenter.onTimeSettingsChanged();
                         presenter.onDurationStateChanged();
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::testProtocolModeChanged, this,
                     [this](const QString &mode) {
                         if (ui->comboBoxTestMode->currentData().toString() != mode) {
                             setTestProtocolMode(mode.toStdString());
                         }

                         presenter.onTimeSettingsChanged();
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::testProtocolProgramChanged, this,
                     [this](const QString &program) {
                         if (ui->comboBoxFormulaTemplate->currentData().toString() == program) {
                             return;
                         }

                         setTestProtocolProgram(program.toStdString());
                     });

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

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::useAngleOfAttackModelChanged, this,
                     [this](bool enabled) {
                         if (ui->checkBoxUseAngleOfAttackModel->isChecked() == enabled) {
                             return;
                         }

                         setUseAngleOfAttackModel(enabled);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::controlPlotChanged, this,
                     [this]() { refreshPlot(); });
}

void ControlChartsTabWidget::populateTestProgramSelection() {
    ui->comboBoxFormulaTemplate->addItem(uiText(localization::ui::customTestProgram), QStringLiteral("custom"));

    ui->comboBoxFormulaTemplate->addItem(uiText(localization::ui::calmTestProgram), QStringLiteral("test1"));
    ui->comboBoxFormulaTemplate->addItem(uiText(localization::ui::maxParametersTestProgram), QStringLiteral("test2"));
    ui->comboBoxFormulaTemplate->addItem(uiText(localization::ui::temporalPerspectiveTestProgram),
                                         QStringLiteral("test3"));
    ui->comboBoxFormulaTemplate->addItem(uiText(localization::ui::attenuatedOscillationTestProgram),
                                         QStringLiteral("attenuated_oscillation"));
}

void ControlChartsTabWidget::populateTestSelectionControls() {
    ui->comboBoxTestMode->addItem(uiText(localization::ui::manualTestMode), QStringLiteral("manual"));
    ui->comboBoxTestMode->addItem(uiText(localization::ui::hybridTestMode), QStringLiteral("hybrid"));
    ui->comboBoxTestMode->addItem(uiText(localization::ui::automaticTestMode), QStringLiteral("automatic"));
}

void ControlChartsTabWidget::updateFormulaEditability(const std::string &program) {
    ui->lineEditFormula->setReadOnly(!domain::testProgramUsesCustomFormula(domain::testProgramFromKey(program)));
}

} // namespace ui
