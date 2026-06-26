#ifndef TRANSPORTSTRINGS_HPP
#define TRANSPORTSTRINGS_HPP

#include <cstddef>
#include <string>

namespace localization::transport {

inline constexpr const char *axisConfigured = "Ось настроена";
inline constexpr const char *axisNotConfigured = "Ось не настроена";
inline constexpr const char *axisEndpointInvalid = "Некорректный адрес или порт оси";
inline constexpr const char *connecting = "Подключение";
inline constexpr const char *connected = "Подключено";
inline constexpr const char *disconnected = "Отключено";
inline constexpr const char *disconnectRequested = "отключение запрошено";
inline constexpr const char *pollingStarted = "обмен телеметрией запущен";
inline constexpr const char *pollingStopped = "обмен телеметрией остановлен";
inline constexpr const char *commandSent = "Команда отправлена";
inline constexpr const char *axisCommandEncodeFailed = "Не удалось закодировать команду оси";
inline constexpr const char *axisCommandWriteFailed = "Не удалось полностью записать пакет команды";
inline constexpr const char *telemetryDecodeFailed = "Не удалось декодировать кадр телеметрии";
inline constexpr const char *telemetryResponseTimeout = "Тайм-аут ответа телеметрии";
inline constexpr const char *socketErrorPrefix = "Ошибка сокета: ";
inline constexpr const char *rxCrcNotConfirmed =
    "RX CRC не подтверждён существующей CRC-функцией; применяется проверка заголовка/длины и восстановление "
    "синхронизации";

inline std::string socketError(const std::string &details) {
    return std::string{socketErrorPrefix} + details;
}

inline std::string discardedRxBytes(std::size_t count) {
    return "отброшены байты RX до следующего заголовка: " + std::to_string(count);
}

inline std::string invalidRxFrameDiscarded() {
    return "невалидный RX-кадр отброшен, выполняется поиск следующего заголовка";
}

} // namespace localization::transport

#endif // TRANSPORTSTRINGS_HPP
