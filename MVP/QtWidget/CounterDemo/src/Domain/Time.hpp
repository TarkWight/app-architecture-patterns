#ifndef TIME_HPP
#define TIME_HPP

namespace domain {

struct DurationMinutes {
    int value{20};
};

struct ElapsedSeconds {
    int value{0};
};

struct RemainigSeconds {
    int value{ 20 * 60 };
};

} // namespace domain

#endif // TIME_HPP
