#include "MainWindowUiAdapter.hpp"

namespace ui {

application::dto::RgbColor MainWindowUiAdapter::toDomainColor(const QColor &color) {
    return application::dto::RgbColor{static_cast<std::uint8_t>(color.red()), static_cast<std::uint8_t>(color.green()),
                                      static_cast<std::uint8_t>(color.blue())};
}

std::string MainWindowUiAdapter::formatElapsed(int elapsedSeconds) {
    const int minutes = elapsedSeconds / 60;
    const int seconds = elapsedSeconds % 60;

    const std::string secondsText = (seconds < 10) ? "0" + std::to_string(seconds) : std::to_string(seconds);

    return std::to_string(minutes) + ":" + secondsText;
}

} // namespace ui
