#ifndef PDFREPORTCONFIG_HPP
#define PDFREPORTCONFIG_HPP

#include <string>

namespace application::dto {

struct PdfReportConfig final {
    std::string title{};
    std::string organization{};
    std::string licenseNumber{};
    std::string address{};
    std::string testType{};
    std::string operatorName{};
    std::string comment{};
    std::string conclusion{};
    std::string result{};
};

} // namespace application::dto

#endif // PDFREPORTCONFIG_HPP
