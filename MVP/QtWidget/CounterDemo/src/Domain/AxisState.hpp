#ifndef AXISSTATE_HPP
#define AXISSTATE_HPP

namespace domain {

enum class AxisState {
    Disconnected, // связи нет
    Connected,    // связь есть, но работа не запущена
    Operating,    // ось выполняет команды / учавствует в испытании
    Stopped,      // ось подключена, но выполнение остановлено
    Error,        // отказ / аварийное состояние
};

} // namespace domain

#endif // AXISSTATE_HPP
