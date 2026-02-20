#include <QApplication>

#include "Infrastructure/InMemoryCounterRepository.hpp"
#include "Infrastructure/QtTextLogger.hpp"
#include "Application/UseCases/SwitchCounterUseCase.hpp"
#include "Presentation/MainPresenter.hpp"
#include "Ui/MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    infrastructure::InMemoryCounterRepository repository;

    std::function<void(const std::string &)> sink;
    infrastructure::QtTextLogger logger([&sink](const std::string &msg) {
        if (sink) {
            sink(msg);
        }
    });

    application::useCases::SwitchCounterUseCase executeSwitcher(repository, logger);
    presentation::MainPresenter presenter(executeSwitcher);

    ui::MainWindow window(presenter);
    sink = [&window](const std::string &msg) { window.appendCommandLog(msg); };

    window.show();
    return app.exec();
}
