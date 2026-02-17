#include "QtTextLogger.hpp"

namespace infrastructure {

QtTextLogger::QtTextLogger(Sink sink) : sink(std::move(sink)) {
}

void QtTextLogger::info(const std::string &message) {
    if (sink) {
        sink(message);
    }
}

} // namespace infrastructure
