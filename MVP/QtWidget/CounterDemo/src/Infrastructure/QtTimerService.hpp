#ifndef QTTIMERSERVICE_HPP
#define QTTIMERSERVICE_HPP

#include <QObject>
#include <QTimer>

#include "../Application/Ports/ITimerService.hpp"

namespace infrastructure {

class QtTimerService final : public QObject, public application::ports::ITimerService {
    Q_OBJECT

  public:
    explicit QtTimerService(QObject *parent = nullptr);

    void start(TickCallback onTick) override;
    void stop() override;

  private:
    QTimer timer;
    TickCallback callback{};
    int elapsedSeconds{0};

    void handleTimeout();
};

} // namespace infrastructure

#endif // QTTIMERSERVICE_HPP
