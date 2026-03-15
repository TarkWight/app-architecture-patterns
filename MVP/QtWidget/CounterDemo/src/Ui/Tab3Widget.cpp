#include "Tab3Widget.hpp"
#include "ui_Tab3Widget.h"

#include <QFileDialog>
#include <QString>

namespace {

QLineEdit *lineEditByIndex(Ui::Tab3Widget *ui, int index) {
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

Tab3Widget::Tab3Widget(presentation::tab3::Tab3Presenter &presenter,
                       infrastructure::SessionStateQtAdapter &sessionAdapter, QWidget *parent)
    : QWidget(parent), ui(new Ui::Tab3Widget), presenter(presenter), sessionAdapter(sessionAdapter) {
    ui->setupUi(this);

    presenter.attachView(*this);

    connectSignals();
    connectSessionSignals();
}

Tab3Widget::~Tab3Widget() {
    presenter.detachView();
    delete ui;
}

void Tab3Widget::setCounterValue(int value) {
    ui->labelCounterValue->setText(QString::number(value));
}

void Tab3Widget::setTimerDurationMinutes(int minutes) {
    ui->spinBoxTimerMinutes->setValue(minutes);
}

void Tab3Widget::setPoemTitle(const std::string &title) {
    ui->lineEditTitle->setText(QString::fromStdString(title));
}

void Tab3Widget::setPoemLine(int index, const std::string &line) {
    auto *lineEdit = lineEditByIndex(ui, index);
    if (lineEdit == nullptr) {
        return;
    }

    lineEdit->setText(QString::fromStdString(line));
}

void Tab3Widget::showExportSuccess(const std::string &filePath) {
    ui->labelExportStatus->setText(QStringLiteral("Exported: %1").arg(QString::fromStdString(filePath)));
}

void Tab3Widget::appendLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

void Tab3Widget::connectSignals() {
    QObject::connect(ui->buttonIncrement, &QPushButton::clicked, this, [this]() { presenter.onIncrementPressed(); });

    QObject::connect(ui->buttonDecrement, &QPushButton::clicked, this, [this]() { presenter.onDecrementPressed(); });

    QObject::connect(ui->buttonReset, &QPushButton::clicked, this, [this]() { presenter.onResetPressed(); });

    QObject::connect(ui->spinBoxTimerMinutes, qOverload<int>(&QSpinBox::valueChanged), this,
                     [this](int value) { presenter.onTimerDurationChanged(value); });

    QObject::connect(ui->lineEditTitle, &QLineEdit::textChanged, this,
                     [this](const QString &text) { presenter.onPoemTitleChanged(text.toStdString()); });

    for (int i = 0; i < 8; ++i) {
        auto *lineEdit = lineEditByIndex(ui, i);
        if (lineEdit == nullptr) {
            continue;
        }

        QObject::connect(lineEdit, &QLineEdit::textChanged, this,
                         [this, i](const QString &text) { presenter.onPoemLineChanged(i, text.toStdString()); });
    }

    QObject::connect(ui->buttonExportPdf, &QPushButton::clicked, this, [this]() {
        const QString filePath = QFileDialog::getSaveFileName(
            this, QStringLiteral("Export PDF"), QStringLiteral("poem-report.pdf"), QStringLiteral("PDF Files (*.pdf)"));

        if (filePath.isEmpty()) {
            return;
        }

        presenter.onExportPdfPressed(filePath.toStdString());
    });
}

void Tab3Widget::connectSessionSignals() {
    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::timerDurationChanged, this,
                     [this](int minutes) {
                         if (ui->spinBoxTimerMinutes->value() == minutes) {
                             return;
                         }

                         ui->spinBoxTimerMinutes->setValue(minutes);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::poemTitleChanged, this,
                     [this](const QString &title) {
                         if (ui->lineEditTitle->text() == title) {
                             return;
                         }

                         ui->lineEditTitle->setText(title);
                     });

    QObject::connect(&sessionAdapter, &infrastructure::SessionStateQtAdapter::poemLineChanged, this,
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
