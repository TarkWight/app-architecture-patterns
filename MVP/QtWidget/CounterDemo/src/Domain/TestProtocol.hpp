#ifndef TESTPROTOCOL_HPP
#define TESTPROTOCOL_HPP

#include <array>
#include <string>
#include <vector>

namespace domain {

struct TestProtocolParameter {
    std::string label{};
    std::string value{};
};

struct TestProtocol {
    std::string title{};
    std::array<std::string, 8> lines{};
    std::string testMode{"manual"};
    std::string testProgram{"test1"};
    std::vector<TestProtocolParameter> droneParameters{};
};

} // namespace domain

#endif // TESTPROTOCOL_HPP
