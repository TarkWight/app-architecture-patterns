#ifndef TESTPROTOCOL_HPP
#define TESTPROTOCOL_HPP

#include <array>
#include <string>

namespace domain {

struct TestProtocol {
    std::string title{};
    std::array<std::string, 8> lines{};
};

} // namespace domain

#endif // TESTPROTOCOL_HPP
