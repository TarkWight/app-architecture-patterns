#include "TestProtocolTabWidget.hpp"
#include "ui_TestProtocolTabWidget.h"

#include <QGridLayout>
#include <QFileDialog>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>

#include <array>

namespace {

QLineEdit *lineEditByIndex(Ui::TestProtocolTabWidget *ui, int index) {
    switch (index) {
    case 0:
        return ui->lineEditLine0;
    case 1:
        return ui->lineEditLine1;
    case 2:
        return ui->lineEditLine2;
    case 3:
        return ui->lineEditLine3;
    case 4:
        return ui->lineEditLine4;
    case 5:
        return ui->lineEditLine5;
    case 6:
        return ui->lineEditLine6;
    case 7:
        return ui->lineEditLine7;
    default:
        return nullptr;
    }
}

} // namespace

namespace ui {

TestProtocolTabWidget::TestProtocolTabWidget(presentation::testProtocolTab::TestProtocolTabPresenter &presenter,
                                             infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent)
    : QWidget(parent), ui(new Ui::TestProtocolTabWidget), presenter(presenter), sessionAdapter(sessionAdapter) {
    ui->setupUi(this);
    setupScrollableContent();
    setupReportFormLabels();
    setupDroneParametersEditor();

    presenter.attachView(*this);

    connectSignals();
    connectSessionSignals();
}

TestProtocolTabWidget::~TestProtocolTabWidget() {
    presenter.detachView();
    delete ui;
}

void TestProtocolTabWidget::setTestProtocolTitle(const std::string &title) {
    const QSignalBlocker blocker{ui->lineEditTitle};
    ui->lineEditTitle->setText(QString::fromStdString(title));
}

void TestProtocolTabWidget::setTestProtocolLine(int index, const std::string &line) {
    auto *lineEdit = lineEditByIndex(ui, index);
    if (lineEdit == nullptr) {
        return;
    }

    const QSignalBlocker blocker{lineEdit};
    lineEdit->setText(QString::fromStdString(line));
}

void TestProtocolTabWidget::setTestProtocolDroneParameters(
    const std::vector<domain::TestProtocolParameter> &parameters) {
    if (droneParametersLayout == nullptr) {
        return;
    }

    while (auto *item = droneParametersLayout->takeAt(0)) {
        if (auto *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    droneParameterEdits.clear();

    constexpr int columns = 2;
    for (int index = 0; index < static_cast<int>(parameters.size()); ++index) {
        const int row = index / columns;
        const int column = (index % columns) * 2;
        const auto &parameter = parameters[static_cast<std::size_t>(index)];

        auto *label = new QLabel(QString::fromStdString(parameter.label), this);
        auto *edit = new QLineEdit(QString::fromStdString(parameter.value), this);
        label->setMinimumWidth(160);
        edit->setMinimumWidth(220);
        droneParameterEdits.push_back(edit);

        droneParametersLayout->addWidget(label, row, column);
        droneParametersLayout->addWidget(edit, row, column + 1);

        QObject::connect(edit, &QLineEdit::textChanged, this, [this, index](const QString &text) {
            presenter.onTestProtocolDroneParameterChanged(index, text.toStdString());
        });
    }
}

void TestProtocolTabWidget::showExportSuccess(const std::string &filePath) {
    ui->labelExportStatus->setText(QStringLiteral("Exported: %1").arg(QString::fromStdString(filePath)));
}

void TestProtocolTabWidget::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void TestProtocolTabWidget::setupScrollableContent() {
    auto *contentWidget = new QWidget(this);
    contentWidget->setLayout(ui->verticalLayoutRoot);
    contentWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(contentWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->addWidget(scrollArea);
}

void TestProtocolTabWidget::setupReportFormLabels() {
    auto *header = new QWidget(this);
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addWidget(new QLabel(QStringLiteral("Данные PDF-протокола, организации и результата"), header));
    headerLayout->addStretch(1);

    loadPdfTomlButton = new QPushButton(QStringLiteral("Загрузить TOML"), header);
    loadPdfTomlButton->setToolTip(QStringLiteral("Подгрузить значения полей протокола из .toml файла"));
    headerLayout->addWidget(loadPdfTomlButton);

    savePdfTomlTemplateButton = new QPushButton(QStringLiteral("Создать шаблон TOML"), header);
    savePdfTomlTemplateButton->setToolTip(QStringLiteral("Сохранить пустой .toml шаблон для полей протокола"));
    headerLayout->addWidget(savePdfTomlTemplateButton);
    ui->verticalLayoutRoot->insertWidget(2, header);

    const std::array<const char *, 8> labels{"Организация", "Номер лицензии", "Адрес",     "ФИО оператора",
                                             "Комментарий", "Заключение",     "Результат", "Резерв"};

    for (int row = 0; row < static_cast<int>(labels.size()); ++row) {
        auto *lineEdit = lineEditByIndex(ui, row);
        if (lineEdit == nullptr) {
            continue;
        }

        ui->gridLayoutTestProtocol->removeWidget(lineEdit);
        if (row == 7) {
            lineEdit->hide();
            continue;
        }

        lineEdit->setMinimumWidth(360);
        ui->gridLayoutTestProtocol->addWidget(
            new QLabel(QString::fromUtf8(labels[static_cast<std::size_t>(row)]), this), row, 0);
        ui->gridLayoutTestProtocol->addWidget(lineEdit, row, 1);
    }

    ui->gridLayoutTestProtocol->setColumnStretch(0, 0);
    ui->gridLayoutTestProtocol->setColumnStretch(1, 1);
}

void TestProtocolTabWidget::setupDroneParametersEditor() {
    auto *group = new QGroupBox(QStringLiteral("Конфигурация БПЛА и параметры теста"), this);
    auto *layout = new QVBoxLayout(group);

    droneParametersLayout = new QGridLayout();
    droneParametersLayout->setHorizontalSpacing(8);
    droneParametersLayout->setVerticalSpacing(6);
    layout->addLayout(droneParametersLayout);
    droneParametersLayout->setColumnStretch(1, 1);
    droneParametersLayout->setColumnStretch(3, 1);

    ui->verticalLayoutRoot->insertWidget(4, group);
}

void TestProtocolTabWidget::connectSignals() {
    QObject::connect(ui->lineEditTitle, &QLineEdit::textChanged, this,
                     [this](const QString &text) { presenter.onTestProtocolTitleChanged(text.toStdString()); });

    for (int i = 0; i < 8; ++i) {
        auto *lineEdit = lineEditByIndex(ui, i);
        if (lineEdit == nullptr) {
            continue;
        }

        QObject::connect(lineEdit, &QLineEdit::textChanged, this, [this, i](const QString &text) {
            presenter.onTestProtocolLineChanged(i, text.toStdString());
        });
    }

    QObject::connect(loadPdfTomlButton, &QPushButton::clicked, this, [this]() {
        const QString filePath =
            QFileDialog::getOpenFileName(this, QStringLiteral("Загрузить данные PDF из TOML"), QStringLiteral(""),
                                         QStringLiteral("TOML Files (*.toml);;All Files (*)"));

        if (filePath.isEmpty()) {
            return;
        }

        presenter.onLoadPdfTomlPressed(filePath.toStdString());
    });

    QObject::connect(savePdfTomlTemplateButton, &QPushButton::clicked, this, [this]() {
        const QString filePath = QFileDialog::getSaveFileName(this, QStringLiteral("Создать шаблон PDF TOML"),
                                                              QStringLiteral("pdf_report.template.toml"),
                                                              QStringLiteral("TOML Files (*.toml);;All Files (*)"));

        if (filePath.isEmpty()) {
            return;
        }

        presenter.onSavePdfTomlTemplatePressed(filePath.toStdString());
    });

    QObject::connect(ui->buttonExportPdf, &QPushButton::clicked, this, [this]() {
        const QString filePath =
            QFileDialog::getSaveFileName(this, QStringLiteral("Export PDF"), QStringLiteral("testProtocol-report.pdf"),
                                         QStringLiteral("PDF Files (*.pdf)"));

        if (filePath.isEmpty()) {
            return;
        }

        presenter.onExportPdfPressed(filePath.toStdString());
    });
}

void TestProtocolTabWidget::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::testProtocolTitleChanged, this,
                     [this](const QString &title) {
                         if (ui->lineEditTitle->text() == title) {
                             return;
                         }

                         setTestProtocolTitle(title.toStdString());
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::testProtocolLineChanged, this,
                     [this](int index, const QString &line) {
                         auto *lineEdit = lineEditByIndex(ui, index);
                         if (lineEdit == nullptr) {
                             return;
                         }

                         if (lineEdit->text() == line) {
                             return;
                         }

                         setTestProtocolLine(index, line.toStdString());
                     });
}

} // namespace ui
