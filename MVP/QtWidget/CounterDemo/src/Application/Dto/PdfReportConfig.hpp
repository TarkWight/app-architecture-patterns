#ifndef PDFREPORTCONFIG_HPP
#define PDFREPORTCONFIG_HPP

#include "../../Domain/TestProtocol.hpp"

#include <string>
#include <vector>

namespace application::dto {

struct PdfReportConfig final {
    std::string title{};
    std::string organization{};
    std::string licenseNumber{};
    std::string address{};
    std::string testMode{};
    std::string testProgram{};
    std::string operatorName{};
    std::string comment{};
    std::vector<domain::TestProtocolParameter> droneParameters{};
    std::string conclusion{};
    std::string result{};
};

} // namespace application::dto

#endif // PDFREPORTCONFIG_HPP
