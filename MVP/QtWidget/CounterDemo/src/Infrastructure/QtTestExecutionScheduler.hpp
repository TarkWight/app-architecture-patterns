#ifndef QTTESTEXECUTIONSCHEDULER_HPP
#define QTTESTEXECUTIONSCHEDULER_HPP

#include <QObject>
#include <QTimer>

#include "../Application/Ports/ITestExecutionScheduler.hpp"

namespace infrastructure {

class QtTestExecutionScheduler final : public QObject, public application::ports::ITestExecutionScheduler {
    Q_OBJECT

  public:
    explicit QtTestExecutionScheduler(QObject *parent = nullptr);

    void start(int initialElapsedSeconds, TickCallback onTick) override;
    void pause() override;
    void resume() override;
    void stop() override;

    bool isRunning() const override;
    bool isPaused() const override;

  private slots:
    void handleTimeout();

  private:
    enum class State { Idle, Running, Paused };

    QTimer timer{};
    TickCallback callback{};
    int elapsedSeconds{0};
    State state{State::Idle};
};

} // namespace infrastructure

#endif // QTTESTEXECUTIONSCHEDULER_HPP