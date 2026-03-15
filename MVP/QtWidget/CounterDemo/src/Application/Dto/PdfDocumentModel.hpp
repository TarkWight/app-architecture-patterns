#ifndef PDFDOCUMENTMODEL_HPP
#define PDFDOCUMENTMODEL_HPP

#include <array>
#include <string>

#include "../../Domain/Plot.hpp"

namespace application::dto {

struct PdfDocumentModel final {
    std::string title{};

    std::array<std::string, 4> firstStanza{};
    domain::PlotModel plot1{};
    std::string plot1Caption{"Рисунок 1"};

    domain::PlotModel plot2{};
    std::string plot2Caption{"Рисунок 2"};
    std::array<std::string, 4> secondStanza{};
};

} // namespace application::dto

#endif // PDFDOCUMENTMODEL_HPP
