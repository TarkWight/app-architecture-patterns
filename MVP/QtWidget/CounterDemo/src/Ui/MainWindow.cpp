#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QString>

namespace ui {

MainWindow::MainWindow(presentation::MainPresenter &presenter, QWidget *parent)
    : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()), presenter(presenter) {
    ui->setupUi(this);

    presenter.attachView(*this);
    connectSignals();
    presenter.onViewReady();
}

MainWindow::~MainWindow() {
    presenter.detachView();
}

void MainWindow::connectSignals() {

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this](int index) { presenter.onTabChanged(index); });

    connect(ui->btnPlus, &QPushButton::clicked, this, [this]() { presenter.onIncrementPressed(); });

    connect(ui->btnMinus, &QPushButton::clicked, this, [this]() { presenter.onDecrementPressed(); });

    connect(ui->btnReset, &QPushButton::clicked, this, [this]() { presenter.onResetPressed(); });
}

void MainWindow::setCounterValue(domain::CounterId id, int value) {

    switch (id.getValue()) {
    case 0:
        ui->labelCounter1->setText(QString::number(value));
        break;
    case 1:
        ui->labelCounter2->setText(QString::number(value));
        break;
    case 2:
        ui->labelCounter3->setText(QString::number(value));
        break;
    }
}

void MainWindow::appendCommandLog(const std::string &text) {
    ui->plainTextEditLog->appendPlainText(QString::fromStdString(text));
}

} // namespace ui
