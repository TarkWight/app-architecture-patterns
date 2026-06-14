#include "QtAppFileLocationProvider.hpp"

#include <QCoreApplication>

namespace infrastructure::configTemplates {

std::filesystem::path QtAppFileLocationProvider::applicationDir() const {
    return std::filesystem::path{QCoreApplication::applicationDirPath().toStdString()};
}

} // namespace infrastructure::configTemplates
