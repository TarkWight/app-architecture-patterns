#include <QApplication>

#include "Application/ApplicationContext.hpp"

#include "Ui/MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ApplicationContext context;

    ui::MainWindow window{ui::MainWindow::Dependencies{.shellPresenter = context.shellPresenter,
                                                       .tab1Presenter = context.tab1Presenter,
                                                       .tab2Presenter = context.tab2Presenter,
                                                       .tab3Presenter = context.tab3Presenter,
                                                       .sessionAdapter = context.sessionAdapter}};

    window.show();

    return app.exec();
}
