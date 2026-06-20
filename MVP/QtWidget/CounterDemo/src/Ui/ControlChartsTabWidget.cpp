#include "ControlChartsTabWidget.hpp"
#include "ui_ControlChartsTabWidget.h"

#include "../Domain/FormulaTemplate.hpp"
#include "../Domain/WindImpact.hpp"

#include <QSignalBlocker>
#include <QString>

namespace ui {

ControlChartsTabWidget::ControlChartsTabWidget(presentation::controlChartsTab::ControlChartsTabPresenter &presenter,
                                               infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent)
    : QWidget(parent), ui(new Ui::ControlChartsTabWidget), presenter(presenter), sessionAdapter(sessionAdapter) {
    ui->setupUi(this);
    ui->doubleSpinBoxBeaufort->setRange(domain::minOperationalBeaufort, domain::maxOperationalBeaufort);
    populateTestSelectionControls();
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

void ControlChartsTabWidget::setReadinessCalculationEnabled(bool enabled) {
    ui->buttonCalculateReadiness->setEnabled(enabled);
}

void ControlChartsTabWidget::setTestProtocolMode(const std::string &mode) {
    const QSignalBlocker blocker{ui->comboBoxTestMode};
    const int index = ui->comboBoxTestMode->findData(QString::fromStdString(mode));
    ui->comboBoxTestMode->setCurrentIndex(index >= 0 ? index : 0);
}

void ControlChartsTabWidget::setTestProtocolProgram(const std::string &program) {
    const QSignalBlocker blocker{ui->comboBoxTestProgram};
    const int index = ui->comboBoxTestProgram->findData(QString::fromStdString(program));
    ui->comboBoxTestProgram->setCurrentIndex(index >= 0 ? index : 0);
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
    plotWidget->setPlot(sessionAdapter.getState().get().control.controlPlot);
}

void ControlChartsTabWidget::showReadinessMessage(const std::string &message) {
    ui->plainTextEditReadinessStatus->setPlainText(QString::fromStdString(message));
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
    QObject::connect(ui->buttonCalculateReadiness, &QPushButton::clicked, this,
                     [this]() { presenter.onReadinessCalculationPressed(); });
    QObject::connect(ui->buttonPickLineColor, &QPushButton::clicked, this, [this]() { emit lineColorRequested(); });
    QObject::connect(ui->comboBoxTestMode, &QComboBox::currentIndexChanged, this, [this]() {
        presenter.onTestProtocolModeChanged(ui->comboBoxTestMode->currentData().toString().toStdString());
    });
    QObject::connect(ui->comboBoxTestProgram, &QComboBox::currentIndexChanged, this, [this]() {
        presenter.onTestProtocolProgramChanged(ui->comboBoxTestProgram->currentData().toString().toStdString());
    });

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
                         if (ui->comboBoxTestProgram->currentData().toString() == program) {
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

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::controlPlotChanged, this,
                     [this]() { refreshPlot(); });
}

void ControlChartsTabWidget::populateFormulaTemplates() {
    ui->comboBoxFormulaTemplate->addItem(QStringLiteral("Своя формула"), QString{});

    for (const auto &formulaTemplate : domain::formulaTemplates) {
        ui->comboBoxFormulaTemplate->addItem(
            QString::fromUtf8(formulaTemplate.title.data(), static_cast<qsizetype>(formulaTemplate.title.size())),
            QString::fromUtf8(formulaTemplate.key.data(), static_cast<qsizetype>(formulaTemplate.key.size())));
    }
}

void ControlChartsTabWidget::populateTestSelectionControls() {
    ui->comboBoxTestMode->addItem(QStringLiteral("Ручное"), QStringLiteral("manual"));
    ui->comboBoxTestMode->addItem(QStringLiteral("Гибридное"), QStringLiteral("hybrid"));
    ui->comboBoxTestMode->addItem(QStringLiteral("Автоматическое"), QStringLiteral("automatic"));

    ui->comboBoxTestProgram->addItem(QStringLiteral("Полет в штиль"), QStringLiteral("test1"));
    ui->comboBoxTestProgram->addItem(QStringLiteral("Определение максимальных параметров"), QStringLiteral("test2"));
    ui->comboBoxTestProgram->addItem(QStringLiteral("Исследование временной перспективы"), QStringLiteral("test3"));
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
