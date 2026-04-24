#ifndef COUNTERSNAPSHOT_HPP
#define COUNTERSNAPSHOT_HPP

namespace application::dto {

struct CountersSnapshot final {
    int tab0{0};
    int telemetryChartsTab{0};
    int controlChartsTab{0};
    int global{0};
};

} // namespace application::dto

#endif // COUNTERSNAPSHOT_HPP
