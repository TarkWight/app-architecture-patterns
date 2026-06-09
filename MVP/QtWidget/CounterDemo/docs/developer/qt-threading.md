# Qt-потоки и асинхронность

В MVP многопоточность не вводится. Приложение предполагает, что основные объекты живут в одном Qt event loop основного
потока.

К этому относятся:

- `SessionState`;
- `SessionStateQtAdapter`;
- presenters;
- application use cases;
- `QtTcpTelemetryClient`;
- `QtTestExecutionScheduler`;
- UI widgets.

## Сетевой слой

`QtTcpTelemetryClient` использует `QTcpSocket` и `QTimer`. Эти объекты асинхронны, но не требуют отдельного worker
thread сами по себе.

Методы клиента должны вызываться из Qt-потока, которому принадлежит объект клиента. В debug-сборке это проверяется
assert-ами thread affinity.

Подключение к стенду асинхронное: вызов `connectAll()` запускает подключение, но не означает, что TCP-соединение уже
установлено. Состояние `Connected` выставляется только после callback-а от socket-а.

## Таймеры

Таймер испытания и polling телеметрии работают через Qt event loop.

Остановка испытания должна останавливать polling телеметрии. Отключение стенда должно останавливать polling timer и
разрывать TCP-соединения.

## Если появятся worker threads

Worker thread не должен напрямую вызывать методы `SessionState`.

Правильная схема для будущего:

```text
worker thread
-> immutable event/snapshot
-> queued call в основной Qt-поток
-> use case или application service
-> SessionState
```

Также нельзя напрямую менять QWidget из worker thread. Все изменения UI должны выполняться в основном Qt-потоке.

## Текущее ограничение

Пока `SessionState::get()` возвращает ссылку, а не копию, вся архитектура должна сохранять single-thread контракт.

Если это ограничение нарушить, возможны:

- чтение частично обновлённого состояния;
- гонки между callbacks и UI;
- уведомления подписчиков не из GUI-потока;
- падения при обращении к QWidget не из основного потока.
