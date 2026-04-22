#include "TestProtocolTabWidget.hpp"
#include "ui_TestProtocolTabWidget.h"

#include <QFileDialog>
#include <QString>

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

    presenter.attachView(*this);

    connectSignals();
    connectSessionSignals();
}

TestProtocolTabWidget::~TestProtocolTabWidget() {
    presenter.detachView();
    delete ui;
}

void TestProtocolTabWidget::setOperatorTestDurationMinutes(int minutes) {
    ui->spinBoxTimerMinutes->setValue(minutes);
}

void TestProtocolTabWidget::setTestProtocolTitle(const std::string &title) {
    ui->lineEditTitle->setText(QString::fromStdString(title));
}

void TestProtocolTabWidget::setTestProtocolLine(int index, const std::string &line) {
    auto *lineEdit = lineEditByIndex(ui, index);
    if (lineEdit == nullptr) {
        return;
    }

    lineEdit->setText(QString::fromStdString(line));
}

void TestProtocolTabWidget::showExportSuccess(const std::string &filePath) {
    ui->labelExportStatus->setText(QStringLiteral("Exported: %1").arg(QString::fromStdString(filePath)));
}

void TestProtocolTabWidget::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void TestProtocolTabWidget::connectSignals() {
    QObject::connect(ui->spinBoxTimerMinutes, qOverload<int>(&QSpinBox::valueChanged), this,
                     [this](int value) { presenter.onOperatorTestDurationChanged(value); });

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
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::operatorTestDurationChanged, this,
                     [this](int minutes) {
                         if (ui->spinBoxTimerMinutes->value() == minutes) {
                             return;
                         }

                         ui->spinBoxTimerMinutes->setValue(minutes);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::testProtocolTitleChanged, this,
                     [this](const QString &title) {
                         if (ui->lineEditTitle->text() == title) {
                             return;
                         }

                         ui->lineEditTitle->setText(title);
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

                         lineEdit->setText(line);
                     });
}

} // namespace ui
