#ifndef COUNTERSNAPSHOT_HPP
#define COUNTERSNAPSHOT_HPP

namespace application::dto {

struct CountersSnapshot final {
    int tab0{0};
    int tab1{0};
    int tab2{0};
    int global{0};
};

} // namespace application::dto

#endif // COUNTERSNAPSHOT_HPP
