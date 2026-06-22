#ifndef MAINWINDOWUIADAPTER_HPP
#define MAINWINDOWUIADAPTER_HPP

#include <QColor>
#include <string>

#include "../Application/Dto/PlotModel.hpp"

namespace ui {

class MainWindowUiAdapter final {
  public:
    static application::dto::RgbColor toDomainColor(const QColor &color);
    static std::string formatElapsed(int elapsedSeconds);
};

} // namespace ui

#endif // MAINWINDOWUIADAPTER_HPP
