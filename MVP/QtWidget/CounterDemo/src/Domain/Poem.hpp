#ifndef POEM_HPP
#define POEM_HPP

#include <array>
#include <string>

namespace domain {

struct Poem {
    std::string title{};
    std::array<std::string, 8> lines{};
};

} // namespace domain

#endif // POEM_HPP
