#include <QApplication>

#include "Application/ApplicationContext.hpp"

#include "Ui/MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ApplicationContext context;

    ui::MainWindow window{
        ui::MainWindow::Dependencies{.shellPresenter = context.shellPresenter,
                                     .telemetryChartsTabPresenter = context.telemetryChartsTabPresenter,
                                     .controlChartsTabPresenter = context.controlChartsTabPresenter,
                                     .testProtocolTabPresenter = context.testProtocolTabPresenter,
                                     .sessionAdapter = context.sessionAdapter}};

    window.show();

    return app.exec();
}
