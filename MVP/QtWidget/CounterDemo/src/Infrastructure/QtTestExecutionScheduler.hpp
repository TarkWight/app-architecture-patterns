#ifndef QTTESTEXECUTIONSCHEDULER_HPP
#define QTTESTEXECUTIONSCHEDULER_HPP

#include <QObject>
#include <QTimer>
#include <cstdint>

#include "../Application/Ports/ITestExecutionScheduler.hpp"

namespace infrastructure {

class QtTestExecutionScheduler final : public QObject, public application::ports::ITestExecutionScheduler {
    Q_OBJECT

  public:
    explicit QtTestExecutionScheduler(QObject *parent = nullptr);
    explicit QtTestExecutionScheduler(int intervalMs, QObject *parent = nullptr);

    void start(int initialElapsedSeconds, TickCallback onTick) override;
    void pause() override;
    void resume() override;
    void stop() override;

    bool isRunning() const override;
    bool isPaused() const override;

  private:
    enum class State { Idle, Running, Paused };

    int intervalMs{1000};
    TickCallback callback{};
    int elapsedSeconds{0};
    std::uint64_t generation{0};
    State state{State::Idle};

    void scheduleNextTick();
    void handleTimeout(std::uint64_t expectedGeneration);
};

} // namespace infrastructure

#endif // QTTESTEXECUTIONSCHEDULER_HPP
