#ifndef MAINWINDOWUIADAPTER_HPP
#define MAINWINDOWUIADAPTER_HPP

#include <QColor>
#include <string>

#include "../Domain/Plot.hpp"

namespace ui {

class MainWindowUiAdapter final {
  public:
    static domain::RgbColor toDomainColor(const QColor &color);
    static std::string formatElapsed(int elapsedSeconds);
};

} // namespace ui

#endif // MAINWINDOWUIADAPTER_HPP
