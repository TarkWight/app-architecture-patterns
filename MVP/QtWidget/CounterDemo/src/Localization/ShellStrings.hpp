#ifndef SHELLSTRINGS_HPP
#define SHELLSTRINGS_HPP

namespace localization::shell {

inline constexpr const char *testExecutionStarted = "Испытание запущено";
inline constexpr const char *testExecutionPaused = "Испытание приостановлено";
inline constexpr const char *testExecutionResumed = "Испытание продолжено";
inline constexpr const char *testExecutionStopped = "Испытание остановлено";

inline constexpr const char *startBlockedStandDisconnected = "Запуск испытания заблокирован: стенд не подключён";
inline constexpr const char *readinessAndPlotBuilt = "Готовность рассчитана, график управляющего воздействия построен";
inline constexpr const char *functionExpressionUpdated = "Формула управляющего воздействия обновлена";
inline constexpr const char *lineColorUpdated = "Цвет линии обновлён";
inline constexpr const char *testTimeSourceUpdated = "Источник времени теста обновлён";
inline constexpr const char *standControlModeUpdated = "Режим управления стендом обновлён";

inline constexpr const char *standConnectionStopped = "Подключение к стенду остановлено";
inline constexpr const char *standConnectionStarted = "Подключение к стенду запущено";
inline constexpr const char *standConnectionFailedPrefix = "Не удалось подключиться к стенду: ";
inline constexpr const char *standConnectionLost = "Связь со стендом потеряна";

inline constexpr const char *standNotConnectedTitle = "Стенд не подключён";
inline constexpr const char *standNotConnectedMessage = "Перед запуском испытания необходимо подключить стенд.";
inline constexpr const char *standConnectionErrorTitle = "Ошибка подключения стенда";
inline constexpr const char *standConnectionLostTitle = "Связь со стендом потеряна";
inline constexpr const char *standConnectionLostMessage =
    "Обмен телеметрией остановлен. Проверьте питание стенда, сеть и состояние тестового сервера или реального стенда.";

inline constexpr const char *dangerousStartConfirmationTitle = "Подтверждение запуска испытания";
inline constexpr const char *readinessFailedStartDetails =
    "Расчёт готовности невозможен. Проведение испытания может быть опасным.";
inline constexpr const char *readinessDangerousStartDetails =
    "Расчёт готовности содержит опасные диагностические сообщения. Проведение испытания может быть опасным.";
inline constexpr const char *continueStartQuestion = " Продолжить запуск?";

inline constexpr const char *pauseButton = "Пауза";
inline constexpr const char *resumeButton = "Продолжить";
inline constexpr const char *connectStandButton = "Подключить стенд";
inline constexpr const char *disconnectStandButton = "Отключить стенд";

inline constexpr const char *standDisconnectedStatus = "Стенд: отключен";
inline constexpr const char *standConfiguredStatus = "Стенд: настроен";
inline constexpr const char *standConnectingStatus = "Стенд: подключение";
inline constexpr const char *standConnectedStatus = "Стенд: подключен";
inline constexpr const char *standPollingStatus = "Стенд: обмен активен";
inline constexpr const char *standDisconnectingStatus = "Стенд: отключение";
inline constexpr const char *standErrorStatus = "Стенд: ошибка связи";

} // namespace localization::shell

#endif // SHELLSTRINGS_HPP
